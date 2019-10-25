// Copyright (c) Cristian Gómez. All rights reserved.

#pragma once

#include "AzureKinectDK.h"
#include "GameFramework/GameModeBase.h"
#include "k4a/k4a.h"
#include "k4abt.h"
#include "AzureKinectDKGameModeBase.generated.h"

UCLASS()
class AZUREKINECTDK_API AAzureKinectDKGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAzureKinectDKGameModeBase();

	/** Handles the Azure Kinect device. */
	k4a_device_t Device;

	/** Handles the body tracking. */
	k4abt_tracker_t Tracker;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the game starts to end
	virtual void BeginDestroy() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void DrawSkeleton(k4abt_joint_t* Joints);
	void DrawBone(k4abt_joint_t Joint1, k4abt_joint_t Joint2);
	FVector RealToPosition(k4a_float3_t Real);
};