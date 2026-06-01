#ifndef CODRIVER_COACH_TEMPLATE_H
#define CODRIVER_COACH_TEMPLATE_H

#include "types.h"

namespace codriver {

class CoachTemplate {
public:
    CoachTemplate();
    ~CoachTemplate();

    CoachMessage generate(const char* segment_id,
                          const char* root_cause,
                          double delta_value,
                          int tier);

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
