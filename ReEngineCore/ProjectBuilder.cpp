#include "ProjectBuilder.h"






extern "C" {
    CORE_API ProjectBuilder* CreateProjectBuilder()
    {
        return new ProjectBuilder();
    }
}
