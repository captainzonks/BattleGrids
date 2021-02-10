#pragma once

#include "Engine/DataTable.h"
#include "BGTypes.generated.h"

USTRUCT(BlueprintType)
struct  FBGTokenBank : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TokenName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ABGToken> TokenClassReference;
};