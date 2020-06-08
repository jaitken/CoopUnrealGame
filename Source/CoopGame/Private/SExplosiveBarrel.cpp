// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 500;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; //only will apply force using FireImpulse()
	RadialForceComp->bIgnoreOwningActor = true;
	ExplosionImpulse = 400;


	DefaultMaterial = CreateDefaultSubobject<UMaterial>(TEXT("DefaultMaterial"));
	ExplodedMaterial = CreateDefaultSubobject<UMaterial>(TEXT("ExplodedMaterial"));

	SetReplicates(true);
	SetReplicateMovement(true);


}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
	MeshComp->SetMaterial(0, DefaultMaterial);

	bExploded = false;

}

void ASExplosiveBarrel::OnRep_Exploded()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	MeshComp->SetMaterial(0, ExplodedMaterial);
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent * OwningHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{

	if (Health <= 0.0f && !bExploded)
	{
		Explode();
		bExploded = true;
		OnRep_Exploded();

	}

}

void ASExplosiveBarrel::Explode()
{
	MeshComp->SetMaterial(0, ExplodedMaterial);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	
	//random x/y direction to launch the barrel
	float yDir = (FMath::Rand() % (int)ExplosionImpulse) - ExplosionImpulse / 2;
	float xDir = (FMath::Rand() % (int)ExplosionImpulse) - ExplosionImpulse / 2;

	MeshComp->AddImpulse(FVector(xDir, yDir, ExplosionImpulse),"None", true);

	RadialForceComp->FireImpulse();
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}
