#include "CWarGas.h"
#include <utility>

std::list<CWarGas *> g_GasList;

CWarGas::CWarGas(const std::string& EffName, CArea *StartArea, std::vector<CArea *> TargetArea, int DurationTime) : StartArea(StartArea), TargetArea(std::move(TargetArea)), DurationTime(DurationTime) {
    ecEffectManager *EffectManager = ecEffectManager::Instance();
    StartPos = new Pos {(float) StartArea->ArmyPos[0], (float) StartArea->ArmyPos[1]};
    gettimeofday(&this->StartTv, nullptr);
    this->StartArea->SetAllArmyPoisoning(3);
    for (CArea *Area : this->TargetArea) {
        GasEff.push_back(EffectManager->AddEffect(EffName.c_str(), true));
        TargetPos.push_back(new Pos {(float) Area->ArmyPos[0], (float) Area->ArmyPos[1]});
        NowPos.push_back(new Pos {StartPos->x, StartPos->y});
        GasEff.back()->FireAt(StartPos->x, StartPos->y, 0);
    }
}

CWarGas::~CWarGas() {
    for (ecEffect *Gas : GasEff)
        delete Gas;
}

void CWarGas::Update() {
    gettimeofday(&this->NowTv, nullptr);
    UseTime = (NowTv.tv_sec - StartTv.tv_sec) * 1000000 + NowTv.tv_usec - StartTv.tv_usec;
    float progress = UseTime / (float) DurationTime;
    if (progress > 0.1 && progress < 1.0) {
        for (int index = 0; index < NowPos.size(); index++) {
            if (progress < 0.9) {
                NowPos[index]->x = StartPos->x + (TargetPos[index]->x - StartPos->x) * (progress - 0.1f) * 1.25f;
                NowPos[index]->y = StartPos->y + (TargetPos[index]->y - StartPos->y) * (progress - 0.1f) * 1.25f;
            } else {
                NowPos[index]->x = TargetPos[index]->x;
                NowPos[index]->y = TargetPos[index]->y;
            }
        }
    } else if (progress >= 1.0) {
        for (CArea *Area : TargetArea)
            Area->SetAllArmyPoisoning(3);
        Clear();
    }
}

void CWarGas::Render() {
    for (int index = 0; index < NowPos.size(); index++)
        GasEff[index]->MoveTo(NowPos[index]->x, NowPos[index]->y, false);
}

void CWarGas::Clear() {
    for (ecEffect *Effect : GasEff)
        Effect->Stop(false);
    IsActive = false;
}
