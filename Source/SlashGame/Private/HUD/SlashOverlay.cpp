// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void USlashOverlay::SetHealthPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetStaminaPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetGoldNumber(int32 Gold)
{
	if (CoinAmount)
	{
		const FString GoldString = FString::Printf(TEXT("%d"), Gold);
		const FText GoldText = FText::FromString(GoldString);
		CoinAmount->SetText(GoldText);
	}
}

void USlashOverlay::SetSoulsNumber(int32 Souls)
{
	if (SoulsAmount)
	{
		const FString SoulsString = FString::Printf(TEXT("%d"), Souls);
		const FText SoulsText = FText::FromString(SoulsString);
		SoulsAmount->SetText(SoulsText);

	}
}
