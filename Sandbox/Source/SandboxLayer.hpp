#pragma once

#include "Vi.hpp"

class SandboxLayer: public  Vi::Layer {
public:
    SandboxLayer();
    ~SandboxLayer() override = default;

    void onAttach() override;
    void onDetach() override;

    void onUpdate(const Vi::Timestep& timestep) override;
    void onEvent(Vi::Event& event) override;
};
