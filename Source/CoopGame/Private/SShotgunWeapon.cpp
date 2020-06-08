// Fill out your copyright notice in the Description page of Project Settings.


#include "SShotgunWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame/CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"


void ASShotgunWeapon::Fire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}


	if (CurrentAmmo > 0)
	{

		//trace a line from pawn eyes to crosshair location(center screen)
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			FVector EyeLocation;
			FRotator EyeRotation;

			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector ShotDirection = EyeRotation.Vector();

			//offset Eyelocation so the line trace starts more in line with muzzlelocation
			EyeLocation = EyeLocation + (ShotDirection * 175);


			//bullet spread
			for (int i = 0; i < PelletCount; i++)
			{

				float HalfRad = FMath::DegreesToRadians(BulletSpread);
				ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

				FVector TraceEnd = EyeLocation + (ShotDirection * 3000);


				FVector TracerEndPoint = TraceEnd;

				EPhysicalSurface SurfaceType = SurfaceType_Default;

				FHitResult Hit;

				if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
				{

					//Blocking hit, process damage
					AActor* HitActor = Hit.GetActor();

					//determine surface type
					SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

					float ActualDamage = BaseDamage;
					if (SurfaceType == SURFACE_FLESHVULNERABLE)
					{
						ActualDamage *= 4.0f;
					}

					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

					PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

					TracerEndPoint = Hit.ImpactPoint;
				}


				PlayFireEffects(TracerEndPoint);



			
				//DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 10.0f, 0, 1.0f);

				LastFireTime = GetWorld()->TimeSeconds;

				if (GetLocalRole() == ROLE_Authority)
				{
					HitScanTrace.TraceTo = TracerEndPoint;
					HitScanTrace.SurfaceType = SurfaceType;
					HitScanTrace.shotCount++;
				}
			}
			CurrentAmmo--;
			

		}

	}
	else
	{
		StartReload();
	}
}