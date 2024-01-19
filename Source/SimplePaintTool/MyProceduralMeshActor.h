// Fill out your copyright notice in the Description page of Project Settings.
#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "MyProceduralMeshActor.generated.h"

UCLASS()
class  AMyProceduralMeshActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyProceduralMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generate")
	FLinearColor ColorStart = FLinearColor::White;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generate")
	FVector2D GridSize = FVector2D(1000, 1000);
	// X 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generate")
	int32 Sublevel_X = 10;
	// Y 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generate")
	int32 Sublevel_Y = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generate")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProceduralMeshComponent* Mesh;

	UFUNCTION(BlueprintCallable)
	void GenerateMesh();
	UFUNCTION()
	void GenerateGrid(TArray<FVector>& InVertices, TArray<int32>& InTriangles, TArray<FVector>& InNormals,
		TArray<FVector2D>& InUV0);

	UFUNCTION(BlueprintCallable)
	void PaintVertexColorWithRT(FLinearColor Color, UTextureRenderTarget2D* RT);
private:
	void CalculateVerticesPos();
	void InitColor();
	
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FColor> vertexColors;
	TArray<FLinearColor> VertexLinearColors;
};