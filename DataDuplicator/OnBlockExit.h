#pragma once

#include <functional>


class OnBlockExit
{
public:
    OnBlockExit(std::function<void(void)> onExit)
        :
        onExit(onExit)
    {
    }
    ~OnBlockExit()
    {
        onExit();
    }
private:
    std::function<void(void)> onExit;
};
