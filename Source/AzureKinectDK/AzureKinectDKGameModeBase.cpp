// Copyright (c) Cristian Gómez. All rights reserved.

#include "AzureKinectDKGameModeBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AAzureKinectDKGameModeBase::AAzureKinectDKGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Device = nullptr;
	Tracker = nullptr;
}

void AAzureKinectDKGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (k4a_device_open(0, &Device) != K4A_RESULT_SUCCEEDED)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to open device"));
	}

	// Start camera. Make sure depth camera is enabled.
	k4a_device_configuration_t DeviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	DeviceConfig.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	DeviceConfig.color_resolution = K4A_COLOR_RESOLUTION_OFF;

	if (k4a_device_start_cameras(Device, &DeviceConfig) != K4A_RESULT_SUCCEEDED)
	{
		UE_LOG(LogTemp, Warning, TEXT("Start K4A cameras failed!"));
	}

	// Get calibration information.
	k4a_calibration_t SensorCalibration;
	if (k4a_device_get_calibration(Device, DeviceConfig.depth_mode, DeviceConfig.color_resolution, &SensorCalibration) != K4A_RESULT_SUCCEEDED)
	{
		UE_LOG(LogTemp, Warning, TEXT("Get depth camera calibration failed!"));
	}

	// Create Body Tracker.
	k4abt_tracker_configuration_t TrackerConfig = K4ABT_TRACKER_CONFIG_DEFAULT;
	if (k4abt_tracker_create(&SensorCalibration, TrackerConfig, &Tracker))
	{
		UE_LOG(LogTemp, Warning, TEXT("Body tracker initialization failed!"));
	}
}

void AAzureKinectDKGameModeBase::BeginDestroy()
{
	Super::BeginDestroy();

	UE_LOG(LogTemp, Warning, TEXT("Finished body tracking processing!"));

	k4abt_tracker_shutdown(Tracker);
	k4abt_tracker_destroy(Tracker);
	k4a_device_stop_cameras(Device);
	k4a_device_close(Device);
}

void AAzureKinectDKGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	k4a_capture_t SensorCapture = nullptr;
	k4a_wait_result_t getCaptureResult = k4a_device_get_capture(Device, &SensorCapture, 0); // timeout_in_ms is set to 0

	if (getCaptureResult == K4A_WAIT_RESULT_SUCCEEDED)
	{
		// timeout_in_ms is set to 0. Return immediately no matter whether the sensorCapture is successfully added
		// to the queue or not.
		k4a_wait_result_t queueCaptureResult = k4abt_tracker_enqueue_capture(Tracker, SensorCapture, 0);

		// Release the sensor capture once it is no longer needed.
		k4a_capture_release(SensorCapture);

		if (queueCaptureResult == K4A_WAIT_RESULT_FAILED)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error! Add capture to tracker process queue failed!"));
			return;
		}
	}
	else if (getCaptureResult != K4A_WAIT_RESULT_TIMEOUT)
	{
		UE_LOG(LogTemp, Warning, TEXT("Get depth capture returned error!"));
		return;
	}

	// Pop Result from Body Tracker.
	k4abt_frame_t BodyFrame = nullptr;
	k4a_wait_result_t PopFrameResult = k4abt_tracker_pop_result(Tracker, &BodyFrame, 0); // timeout_in_ms is set to 0.
	if (PopFrameResult == K4A_WAIT_RESULT_SUCCEEDED) // If successfully, get a body tracking result and process the result here.
	{
		size_t NumBodies = k4abt_frame_get_num_bodies(BodyFrame);
		UE_LOG(LogTemp, Warning, TEXT("Bodies detected: %d"), NumBodies);

		for (size_t Index = 0; Index < NumBodies; Index++)
		{
			k4abt_body_t Body;
			if (k4abt_frame_get_body_skeleton(BodyFrame, Index, &Body.skeleton) != K4A_RESULT_SUCCEEDED)
			{
				UE_LOG(LogTemp, Warning, TEXT("Get Skeleton from body frame failed"));
				return;
			}
			// Flush debug lines.
			FlushPersistentDebugLines(GetWorld());

			// Draw skeleton with debug lines.
			DrawSkeleton(Body.skeleton.joints);
		}
		// Release the body frame once it has been use it.
		k4abt_frame_release(BodyFrame);
	}
}

void AAzureKinectDKGameModeBase::DrawSkeleton(k4abt_joint_t* Joints)
{
	if (Joints == nullptr) return;

	DrawBone(Joints[K4ABT_JOINT_PELVIS], Joints[K4ABT_JOINT_SPINE_NAVAL]);
	DrawBone(Joints[K4ABT_JOINT_SPINE_NAVAL], Joints[K4ABT_JOINT_SPINE_CHEST]);
	DrawBone(Joints[K4ABT_JOINT_SPINE_CHEST], Joints[K4ABT_JOINT_CLAVICLE_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_SPINE_CHEST], Joints[K4ABT_JOINT_NECK]);
	DrawBone(Joints[K4ABT_JOINT_NECK], Joints[K4ABT_JOINT_HEAD]);
	DrawBone(Joints[K4ABT_JOINT_HEAD], Joints[K4ABT_JOINT_NOSE]);
	DrawBone(Joints[K4ABT_JOINT_NOSE], Joints[K4ABT_JOINT_EAR_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_NOSE], Joints[K4ABT_JOINT_EAR_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_NOSE], Joints[K4ABT_JOINT_EYE_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_NOSE], Joints[K4ABT_JOINT_EAR_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_SPINE_CHEST], Joints[K4ABT_JOINT_CLAVICLE_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_CLAVICLE_RIGHT], Joints[K4ABT_JOINT_SHOULDER_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_SHOULDER_RIGHT], Joints[K4ABT_JOINT_ELBOW_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_ELBOW_RIGHT], Joints[K4ABT_JOINT_WRIST_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_WRIST_RIGHT], Joints[K4ABT_JOINT_HAND_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_WRIST_RIGHT], Joints[K4ABT_JOINT_THUMB_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_HAND_RIGHT], Joints[K4ABT_JOINT_HANDTIP_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_CLAVICLE_LEFT], Joints[K4ABT_JOINT_SHOULDER_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_SHOULDER_LEFT], Joints[K4ABT_JOINT_ELBOW_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_ELBOW_LEFT], Joints[K4ABT_JOINT_WRIST_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_WRIST_LEFT], Joints[K4ABT_JOINT_HAND_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_WRIST_LEFT], Joints[K4ABT_JOINT_THUMB_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_HAND_LEFT], Joints[K4ABT_JOINT_HANDTIP_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_PELVIS], Joints[K4ABT_JOINT_HIP_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_HIP_RIGHT], Joints[K4ABT_JOINT_KNEE_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_KNEE_RIGHT], Joints[K4ABT_JOINT_ANKLE_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_ANKLE_RIGHT], Joints[K4ABT_JOINT_FOOT_RIGHT]);
	DrawBone(Joints[K4ABT_JOINT_PELVIS], Joints[K4ABT_JOINT_HIP_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_HIP_LEFT], Joints[K4ABT_JOINT_KNEE_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_KNEE_LEFT], Joints[K4ABT_JOINT_ANKLE_LEFT]);
	DrawBone(Joints[K4ABT_JOINT_ANKLE_LEFT], Joints[K4ABT_JOINT_FOOT_LEFT]);
}

void AAzureKinectDKGameModeBase::DrawBone(k4abt_joint_t Joint1, k4abt_joint_t Joint2)
{
	DrawDebugLine(GetWorld(), RealToPosition(Joint1.position), RealToPosition(Joint2.position),
		FColor::MakeRedToGreenColorFromScalar((Joint1.confidence_level + Joint2.confidence_level) * 0.5f),
		true, -1, 0, 4);
}

FVector AAzureKinectDKGameModeBase::RealToPosition(k4a_float3_t Real)
{
	return FVector(-Real.xyz.x, Real.xyz.y, Real.xyz.z) * 0.1f;
}