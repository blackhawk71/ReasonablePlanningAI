// Troll Purse. All rights reserved.


#include "Composer/ActionTasks/RpaiActionTask_GameplayTaskBase.h"
#include "Core/RpaiTypes.h"
#include "GameplayTasksComponent.h"
#include "Tasks/AITask.h"
#include "AIController.h"
#include "AITypes.h"

FActionTaskGameplayTaskBaseMemory::FActionTaskGameplayTaskBaseMemory()
	: AITask(nullptr)
{

}

URpaiActionTask_GameplayTaskBase::URpaiActionTask_GameplayTaskBase()
{
	bCompleteAfterStart = false;
	ActionTaskMemoryStructType = FActionTaskGameplayTaskBaseMemory::StaticStruct();
}

void URpaiActionTask_GameplayTaskBase::ReceiveCancelActionTask_Implementation(AAIController* ActionInstigator, URpaiState* CurrentState, FRpaiMemoryStruct ActionMemory, AActor* ActionTargetActor, UWorld* ActionWorld)
{
	if (!bCompleteAfterStart)
	{
		UAITask* TheTask = ActionMemory.Get<FActionTaskGameplayTaskBaseMemory>()->AITask;
		if (TheTask != nullptr && !TheTask->IsFinished())
		{
			TheTask->ExternalCancel();
		}
	}
}

UGameplayTasksComponent* URpaiActionTask_GameplayTaskBase::GetGameplayTasksComponent(const UGameplayTask& Task) const
{
	const UAITask* AITask = Cast<const UAITask>(&Task);
	return (AITask && AITask->GetAIController()) ? AITask->GetAIController()->GetGameplayTasksComponent(Task) : Task.GetGameplayTasksComponent();
}

AActor* URpaiActionTask_GameplayTaskBase::GetGameplayTaskOwner(const UGameplayTask* Task) const
{
	if (Task == nullptr)
	{
		return nullptr;
	}

	const UAITask* AITask = Cast<const UAITask>(Task);
	if (AITask)
	{
		return AITask->GetAIController();
	}

	const UGameplayTasksComponent* TasksComponent = Task->GetGameplayTasksComponent();
	return TasksComponent ? TasksComponent->GetGameplayTaskOwner(Task) : nullptr;
}

AActor* URpaiActionTask_GameplayTaskBase::GetGameplayTaskAvatar(const UGameplayTask* Task) const
{
	if (Task == nullptr)
	{
		return nullptr;
	}

	const UAITask* AITask = Cast<const UAITask>(Task);
	if (AITask)
	{
		return AITask->GetAIController() ? AITask->GetAIController()->GetPawn() : nullptr;
	}

	const UGameplayTasksComponent* TasksComponent = Task->GetGameplayTasksComponent();
	return TasksComponent ? TasksComponent->GetGameplayTaskAvatar(Task) : nullptr;
}

uint8 URpaiActionTask_GameplayTaskBase::GetGameplayTaskDefaultPriority() const
{
	return static_cast<uint8>(EAITaskPriority::AutonomousAI);
}

void URpaiActionTask_GameplayTaskBase::OnGameplayTaskInitialized(UGameplayTask& Task)
{
	const UAITask* AITask = Cast<const UAITask>(&Task);
	if (AITask && (AITask->GetAIController() == nullptr))
	{
		UE_LOG(LogRpai, Error, TEXT("Missing AIController in AITask %s"), *AITask->GetName());
	}
}

void URpaiActionTask_GameplayTaskBase::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	UAITask* AITask = Cast<UAITask>(&Task);
	if (AITask && AITask->GetAIController() && AITask->GetState() != EGameplayTaskState::Paused)
	{
		// UMMMMMM.....
		//auto StateKey = TaskToState.FindKey(AITask);
		//if (StateKey != nullptr)
		//{
		//	//naughty
		//	CompleteActionTask(AITask->GetAIController(), const_cast<URpaiState*>(*StateKey));
		//}
	}
}

void URpaiActionTask_GameplayTaskBase::StartTask(URpaiState* CurrentState, UAITask* TaskToStart, FRpaiMemoryStruct ActionMemory)
{
	ActionMemory.Get<FActionTaskGameplayTaskBaseMemory>()->AITask = TaskToStart;
	TaskToStart->ReadyForActivation();
	if (TaskToStart->GetState() == EGameplayTaskState::Finished)
	{
		CompleteActionTask(Cast<AAIController>(GetGameplayTaskOwner(TaskToStart)), CurrentState, ActionMemory);
	}
}
