#pragma once

#include "ReflectionMacro.h"

REFCLASS()
class ReflectionTest {
public:

    REFMETHOD()
    void ShowDamage();

    ReflectionTest() {};
    ~ReflectionTest() {};
private:

    REFVARIABLE()
    bool IsThisMe;
};
