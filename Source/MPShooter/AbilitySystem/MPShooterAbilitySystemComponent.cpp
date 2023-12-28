// Created by Capralkein


#include "MPShooterAbilitySystemComponent.h"

#include "AbilitySystemLog.h"
#include "MPShooterGameplayAbility.h"
#include "Attributes/MPShooterAttributeSet.h"

void UMPShooterAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();

	if (!IsValid(InOwnerActor) || !ActorInfo)
	{
		ABILITY_LOG(Error, TEXT("[MPShooterAbilitySystemComponent] Can't init ability actor info for owner <%s> - not valid actor info!"), *GetNameSafe(InOwnerActor));
		return;
	}

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		// Notify all abilities that a new pawn avatar has been set
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			UMPShooterGameplayAbility* AbilityCDO = CastChecked<UMPShooterGameplayAbility>(AbilitySpec.Ability);

			if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
			{
				TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
				for (UGameplayAbility* AbilityInstance : Instances)
				{
					UMPShooterGameplayAbility* MPShooterAbilityInstance = CastChecked<UMPShooterGameplayAbility>(AbilityInstance);
					MPShooterAbilityInstance->OnPawnAvatarSet();
				}
			}
			else
			{
				AbilityCDO->OnPawnAvatarSet();
			}
		}

		TryActivateAbilitiesOnSpawn();
	}
}

void UMPShooterAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UMPShooterAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UMPShooterAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UMPShooterAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	
	// Process all abilities that activate when the input is held.
	for (const auto& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UMPShooterGameplayAbility* AbilityCDO = CastChecked<UMPShooterGameplayAbility>(AbilitySpec->Ability);

				if (AbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const auto& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UMPShooterGameplayAbility* AbilityCDO = CastChecked<UMPShooterGameplayAbility>(AbilitySpec->Ability);

					if (AbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	
	for (const auto& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility_Internal(AbilitySpecHandle);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	//
	// Clear the cached ability handles.
	//
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UMPShooterAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UMPShooterAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UMPShooterAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.IsActive())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UMPShooterAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);
	
	if (Spec.IsActive())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UMPShooterAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void UMPShooterAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	if (const UMPShooterGameplayAbility* MPShooterAbility = Cast<const UMPShooterGameplayAbility>(Ability))
	{
		MPShooterAbility->OnAbilityFailedToActivate(FailureReason);
	}	
}

bool UMPShooterAbilitySystemComponent::TryActivateAbility_Internal(FGameplayAbilitySpecHandle AbilityToActivate,
														bool bAllowRemoteActivation)
{
	FGameplayTagContainer FailureTags;
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate);
	if (!Spec)
	{
		ABILITY_LOG(Warning, TEXT("TryActivateAbility called with invalid Handle"));
		return false;
	}

	// don't activate abilities that are waiting to be removed
	if (Spec->PendingRemove || Spec->RemoveAfterActivation)
	{
		return false;
	}

	UGameplayAbility* Ability = Spec->Ability;

	if (!Ability)
	{
		ABILITY_LOG(Warning, TEXT("TryActivateAbility called with invalid Ability"));
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();

	// make sure the ActorInfo and then Actor on that FGameplayAbilityActorInfo are valid, if not bail out.
	if (ActorInfo == nullptr || !ActorInfo->OwnerActor.IsValid() || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

		
	const ENetRole NetMode = ActorInfo->AvatarActor->GetLocalRole();

	// This should only come from button presses/local instigation (AI, etc).
	if (NetMode == ROLE_SimulatedProxy)
	{
		return false;
	}

	bool bIsLocal = AbilityActorInfo->IsLocallyControlled();

	// Check to see if this a local only or server only ability, if so either remotely execute or fail
	if (!bIsLocal && (Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalOnly || Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted))
	{
		if (bAllowRemoteActivation)
		{
			ClientTryActivateAbility(AbilityToActivate);
			return true;
		}

		ABILITY_LOG(Log, TEXT("Can't activate LocalOnly or LocalPredicted ability %s when not local."), *Ability->GetName());
		return false;
	}

	if (NetMode != ROLE_Authority && (Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::ServerOnly || Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::ServerInitiated))
	{
		if (bAllowRemoteActivation)
		{
			FGameplayTagContainer SourceTags;
			GetOwnedGameplayTags(SourceTags);
			
			if (Ability->CanActivateAbility(AbilityToActivate, ActorInfo, &SourceTags, nullptr, &FailureTags))
			{
				// No prediction key, server will assign a server-generated key
				CallServerTryActivateAbility(AbilityToActivate, Spec->InputPressed, FPredictionKey());
				return true;
			}
			else
			{
				NotifyAbilityFailed(AbilityToActivate, Ability, FailureTags);
				return false;
			}
		}

		ABILITY_LOG(Log, TEXT("Can't activate ServerOnly or ServerInitiated ability %s when not the server."), *Ability->GetName());
		return false;
	}

	FGameplayTagContainer SourceTags;
	GetOwnedGameplayTags(SourceTags);

	FGameplayEventData FakeEventData;
	FakeEventData.InstigatorTags = SourceTags;

	return InternalTryActivateAbility(AbilityToActivate, FPredictionKey(), nullptr, nullptr, &FakeEventData);
}

void UMPShooterAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		const UMPShooterGameplayAbility* AbilityCDO = CastChecked<UMPShooterGameplayAbility>(AbilitySpec.Ability);
		AbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
	}
}

UMPShooterAttributeSet* UMPShooterAbilitySystemComponent::GetOrCreateAttributeSet(
	TSubclassOf<UMPShooterAttributeSet> AttributeClass)
{
	auto FindAttributeSet = [this](const TSubclassOf<UMPShooterAttributeSet> Class)-> UAttributeSet*
	{
		for (UAttributeSet* Set : GetSpawnedAttributes())
		{
			if (Set && Set->IsA(Class))
			{
				return Set;
			}
		}

		return nullptr;
	};
	
	AActor* OwningActor = GetOwner();
	UMPShooterAttributeSet* MyAttributes = nullptr;
	if (OwningActor && AttributeClass)
	{
		MyAttributes = Cast<UMPShooterAttributeSet>(FindAttributeSet(AttributeClass));
		if (!MyAttributes)
		{
			UMPShooterAttributeSet* Attributes = NewObject<UMPShooterAttributeSet>(OwningActor, AttributeClass);
			AddSpawnedAttribute(Attributes);
			MyAttributes = Attributes;
		}
	}

	return MyAttributes;
}

void UMPShooterAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc,
															bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UMPShooterGameplayAbility* AbilityCDO = CastChecked<UMPShooterGameplayAbility>(AbilitySpec.Ability);

		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// Cancel all the spawned instances, not the CDO.
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				UMPShooterGameplayAbility* MPSAbilityInstance = CastChecked<UMPShooterGameplayAbility>(AbilityInstance);

				if (!ShouldCancelFunc(MPSAbilityInstance, AbilitySpec.Handle))
					continue;
				
				if (MPSAbilityInstance->CanBeCanceled())
				{
					MPSAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), MPSAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
				}
				else
				{
					ABILITY_LOG(Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *MPSAbilityInstance->GetName());
				}
			}
		}
		else
		{
			// Cancel the non-instanced ability CDO.
			if (ShouldCancelFunc(AbilityCDO, AbilitySpec.Handle))
			{
				// Non-instanced abilities can always be canceled.
				check(AbilityCDO->CanBeCanceled());
				AbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
	}
}

FOnGameplayEffectTagCountChanged& UMPShooterAbilitySystemComponent::RegisterBlockedGameplayTagEvent(FGameplayTag Tag,
	EGameplayTagEventType::Type EventType)
{
	return BlockedAbilityTags.RegisterGameplayTagEvent(Tag, EventType);
}

void UMPShooterAbilitySystemComponent::UnregisterBlockedGameplayTagEvent(FDelegateHandle DelegateHandle,
	FGameplayTag Tag, EGameplayTagEventType::Type EventType)
{
	BlockedAbilityTags.RegisterGameplayTagEvent(Tag, EventType).Remove(DelegateHandle);
}

void UMPShooterAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	const TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}
