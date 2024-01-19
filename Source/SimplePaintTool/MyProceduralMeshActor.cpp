// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProceduralMeshActor.h"
#include "Engine/TextureRenderTarget2D.h"

// Sets default values
AMyProceduralMeshActor::AMyProceduralMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("Mesh"));
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void AMyProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMyProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AMyProceduralMeshActor::GenerateMesh()
{
	if (GridSize.X <= 0 || GridSize.Y <= 0 || Sublevel_X < 1 || Sublevel_Y < 1) return;
	Mesh->ClearAllMeshSections();

	// Mesh buffers
	vertices.Empty();
	triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();
	
	CalculateVerticesPos();
	InitColor();
	
	GenerateGrid(vertices, triangles, normals, UV0);
	Mesh->CreateMeshSection(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);
	Mesh->SetMaterial(0, Material);
}

void AMyProceduralMeshActor::GenerateGrid(
	TArray<FVector>& InVertices,
	TArray<int32>& InTriangles,
	TArray<FVector>& InNormals, 
	TArray<FVector2D>& InUV0)
{
	int32 VertexIndex = 0;

	for (int X = 0; X < Sublevel_X + 1; X++)
	{
		for (int Y = 0; Y < Sublevel_Y + 1; Y++)
		{
			// UV
			FVector2D uv = FVector2D((float)X / (float)Sublevel_X, (float)Y / (float)Sublevel_Y);
			InUV0.Add(uv);
			// Once we've created enough verts we can start adding polygons
			if (X > 0 && Y > 0)
			{
				int32 bTopRightIndex = (X * (Sublevel_Y + 1)) + Y;
				int32 bTopLeftIndex = bTopRightIndex - 1;
				int32 pBottomRightIndex = ((X - 1) * (Sublevel_Y + 1)) + Y;
				int32 pBottomLeftIndex = pBottomRightIndex - 1;

				// Now create two triangles from those four vertices
				// The order of these (clockwise/counter-clockwise) dictates which way the normal will face. 
				InTriangles.Add(pBottomLeftIndex);
				InTriangles.Add(bTopRightIndex);
				InTriangles.Add(bTopLeftIndex);

				InTriangles.Add(pBottomLeftIndex);
				InTriangles.Add(pBottomRightIndex);
				InTriangles.Add(bTopRightIndex);
			}
			VertexIndex++;
		}
	}

	// normal
	for (int X = 0; X < Sublevel_X + 1; X++)
	{
		for (int Y = 0; Y < Sublevel_Y + 1; Y++)
		{
			int32 c = (X * (Sublevel_Y + 1)) + Y;
			int32 centerUp = c + Sublevel_Y + 1;
			int32 centerBottom = c - (Sublevel_Y + 1);
			int32 centerRight = c + 1;
			if (centerRight > ((X * (Sublevel_Y + 1)) + Sublevel_Y))
			{
				centerRight = -1;
			}
			int32 centerLeft = c - 1;
			if (centerLeft < (X * (Sublevel_Y + 1)))
			{
				centerLeft = -1;
			}
			int32 centerUpRight = centerUp + 1;
			if (centerUpRight > (X + 1) * (Sublevel_Y + 1) + Sublevel_Y)
			{
				centerUpRight = -1;
			}
			int32 centerBottomLeft = centerBottom - 1;
			if (centerBottomLeft < (X - 1) * (Sublevel_Y + 1))
			{
				centerBottomLeft = -1;
			}

			int32 indexs[6] = { centerUp,centerUpRight,centerRight,centerBottom,centerBottomLeft,centerLeft };
			TArray<FVector> vers;
			// get all the normal of triangles that using the current vertex
			for (int i = 0; i < 6; i++)
			{
				int32 NextIndex = (i + 1 >= 6) ? 0 : (i + 1);
				if (indexs[i] >= 0 && indexs[i] < InVertices.Num() && indexs[NextIndex] >= 0 && indexs[NextIndex] < InVertices.Num())
				{
					FVector a = (InVertices[indexs[i]] - InVertices[c]).GetUnsafeNormal();
					FVector b = (InVertices[indexs[NextIndex]] - InVertices[c]).GetUnsafeNormal();
					vers.Add(FVector::CrossProduct(a, b).GetUnsafeNormal());
					break;
				}
			}

			// get the average Vector
			FVector NormalSum = FVector(0, 0, 1);
			for (auto& i : vers)
			{
				NormalSum += i;
			}
			InNormals.Add(NormalSum / vers.Num());
		}
	}
}

void AMyProceduralMeshActor::PaintVertexColorWithRT(FLinearColor Color, UTextureRenderTarget2D* RT)
{
	TArray<FColor> OutputBuffer;
	if(RT)
	{
		FTextureRenderTargetResource* ResourceRt = RT->GameThread_GetRenderTargetResource();
		if(ResourceRt)
		{
			ResourceRt->ReadPixels(OutputBuffer);
			if(OutputBuffer.Num() == RT->SizeX * RT->SizeY)
			{
				for (int X = 0; X < Sublevel_X + 1; X++)
				{
					for (int Y = 0; Y < Sublevel_Y + 1; Y++)
					{
						int U = X * 1.f / (Sublevel_X + 1) * RT->SizeX + 0.5;
						int V = Y * 1.f / (Sublevel_Y + 1) * RT->SizeY + 0.5;
						if(OutputBuffer.IsValidIndex(V *RT->SizeY + U)
							&&OutputBuffer[V *RT->SizeY + U].R > 0)
						{
							float lerpA = OutputBuffer[V *RT->SizeY + U].R / 255.f;
							FLinearColor CurColor = VertexLinearColors[X*(Sublevel_X + 1) + Y];
							
							FLinearColor FillColor =  CurColor + lerpA *(Color - CurColor);
							
							VertexLinearColors[X*(Sublevel_X + 1) + Y] = FillColor;
							//UE_LOG(LogTemp,Warning,TEXT("Alpha = %f"), lerpA);
						}
					}
				}
				Mesh->UpdateMeshSection_LinearColor(0, vertices, normals, UV0, VertexLinearColors, tangents, false);
			}
		}
	}
}

void AMyProceduralMeshActor::CalculateVerticesPos()
{
	FVector2D SectionSize = FVector2D(GridSize.X / Sublevel_X, GridSize.Y / Sublevel_Y);

	for (int X = 0; X < Sublevel_X + 1; X++)
	{
		for (int Y = 0; Y < Sublevel_Y + 1; Y++)
		{
			vertices.Add(FVector(X * SectionSize.X, Y*SectionSize.Y, 0));
		}
	}
}

void AMyProceduralMeshActor::InitColor()
{
	vertexColors.Empty();
	VertexLinearColors.Empty();
	for (int X = 0; X < Sublevel_X + 1; X++)
	{
		for (int Y = 0; Y < Sublevel_Y + 1; Y++)
		{
			vertexColors.Add(ColorStart.ToFColor(false));
			VertexLinearColors.Add(ColorStart);
		}
	}
}
