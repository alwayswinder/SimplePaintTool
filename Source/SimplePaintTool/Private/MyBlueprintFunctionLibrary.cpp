// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"

bool UMyBlueprintFunctionLibrary::WriteRTtoTexture(UTexture2D* InTex, UTextureRenderTarget2D* InRt, FLinearColor InColor)
{
	if(InRt && InTex)
	{
		FTextureRenderTargetResource* ResourceRt = InRt->GameThread_GetRenderTargetResource();
		uint8* Pixels = new uint8[InRt->SizeX * InRt->SizeY * 4];
		TArray<FColor> OutputBuffer;

		TArray<FColor> OldColor = ReadColorFromTexture(InTex);
		
		if(ResourceRt)
		{
			ResourceRt->ReadPixels(OutputBuffer);
			if(OutputBuffer.Num() == InRt->SizeX * InRt->SizeY)
			{
				for (int32 y = 0; y < InRt->SizeY; y++)
				{
					for (int32 x = 0; x < InRt->SizeX; x++)
					{
						int U = x * 1.f / (InRt->SizeX + 1) * InRt->SizeX + 1;
						int V = y * 1.f / (InRt->SizeY + 1) * InRt->SizeY + 1;
						
						float lerpA = OutputBuffer[V * InRt->SizeY + U].R / 255.f;
						int32 curPixelIndex = ((y * InRt->SizeX) + x);

						FColor PixelColor = OldColor[curPixelIndex];
						FColor FillColor = InColor.ToFColor(false);

						Pixels[4 * curPixelIndex] = PixelColor.B + lerpA *(FillColor.B - PixelColor.B);
						Pixels[4 * curPixelIndex + 1] = PixelColor.G + lerpA *(FillColor.G - PixelColor.G);
						Pixels[4 * curPixelIndex + 2] = PixelColor.R + lerpA *(FillColor.R - PixelColor.R);
						Pixels[4 * curPixelIndex + 3] = PixelColor.A + lerpA *(FillColor.A - PixelColor.A);
					}
				}
			}
		}
		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		InTex->GetPlatformData()->Mips.Add(Mip);
		Mip->SizeX = InRt->SizeX;
		Mip->SizeY = InRt->SizeY;
	
		Mip->BulkData.Lock(LOCK_READ_WRITE);
		uint8* TextureData = (uint8*)Mip->BulkData.Realloc(InRt->SizeX * InRt->SizeY * 4);
		FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * InRt->SizeX * InRt->SizeY * 4);
		Mip->BulkData.Unlock();
	
		InTex->Source.Init(InRt->SizeX, InRt->SizeY, 1, 1, ETextureSourceFormat::TSF_BGRA8, Pixels);
		InTex->UpdateResource();
	
		InTex->GetPackage()->MarkPackageDirty();
		delete[] Pixels;
		return true;
	}
	return false;
}

TArray<FColor> UMyBlueprintFunctionLibrary::ReadColorFromTexture(UTexture2D* InTex)
{
	TArray<FColor> RtColors;
	
	TextureCompressionSettings OldCompressionSettings = InTex->CompressionSettings;
	TextureMipGenSettings OldMipGenSettings = InTex->MipGenSettings;
	bool OldSRGB = InTex->SRGB;

	InTex->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	InTex->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	InTex->SRGB = false;
	InTex->UpdateResource();

	const FColor* FormatedImageData = static_cast<const FColor*>(InTex->GetPlatformData()->Mips[0].BulkData.LockReadOnly());

	for (int32 Y = 0; Y < InTex->GetPlatformData()->SizeY; Y++)
	{
		for (int32 X = 0; X < InTex->GetPlatformData()->SizeX; X++)
		{
			FColor PixelColor = FormatedImageData[Y * InTex->GetPlatformData()->SizeX + X];
			//做若干操作
			RtColors.Add(PixelColor);
		}
	}

	InTex->GetPlatformData()->Mips[0].BulkData.Unlock();

	InTex->CompressionSettings = OldCompressionSettings;
	InTex->MipGenSettings = OldMipGenSettings;
	InTex->SRGB = OldSRGB;
	InTex->UpdateResource();
	
	return RtColors;
}
