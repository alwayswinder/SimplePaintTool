// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEPAINTTOOL_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category="Texture")
	static bool WriteRTtoTexture(UTexture2D* InTex, UTextureRenderTarget2D* InRt, FLinearColor InColor);

	UFUNCTION(BlueprintCallable, Category="Texture")
	static TArray<FColor> ReadColorFromTexture(UTexture2D* InTex);
};
