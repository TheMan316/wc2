#ifndef WORLDCONQUEROR2_CWARGAS_H
#define WORLDCONQUEROR2_CWARGAS_H

#include "../easytech/includes/CScene.h"

struct Pos {
    float x, y;
};

class CWarGas {
public:
    CWarGas(const std::string& EffName, CArea *StartArea, std::vector<CArea *> TargetArea, int DurationTime);
    ~CWarGas();
    void Update();
    void Render();
    void Clear();
    bool IsActive = true;

private:
    std::vector<ecEffect *> GasEff;
    CArea *StartArea;
    std::vector<CArea *> TargetArea;
    Pos *StartPos;
    std::vector<Pos *> TargetPos;
    std::vector<Pos *> NowPos;
    int DurationTime;
    timeval StartTv{};
    timeval NowTv{};
    u_long UseTime = 0;
};

extern std::list<CWarGas *> g_GasList;

#endif //WORLDCONQUEROR2_CWARGAS_H
