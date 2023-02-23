#pragma once

#include <Vi.hpp>

class SandboxLayer: public Vi::Layer {
public:
    SandboxLayer(): Layer("SandboxLayer") {}
    ~SandboxLayer() override = default;

    void onAttach() override {}
    void onDetach() override {}

    void onUpdate() override {}

};

