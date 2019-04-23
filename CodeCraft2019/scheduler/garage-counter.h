#ifndef GARAGE_COUNTER
#define GARAGE_COUNTER

#include "sim-scenario.h"
#include "callback.h"

/* for keeping number of cars in garage in balance */
class GarageCounter
{
public:
    void Initialize(SimScenario& scenario);
    void Update(const int& time, SimScenario& scenario);
    void HandleBeforeGarageDispatch(const int& time, SimScenario& scenario);
    bool CanDispatch(const int& time, SimScenario& scenario, SimCar* car) const;
    void NotifyDispatch(const int& garageId);

private:
    bool m_isScheduling;

    int GetCangoCarsN(const int& time, const int& garageId) const;

    /* quantity limit */
    std::vector<double> m_dispatchCarsNInEachTime;
    std::vector<int> m_carsN;
    std::vector<int> m_leftCarsN;

    /* evaluation */
    std::vector< std::list<SimCar*> > m_bestCars;
};

#endif
