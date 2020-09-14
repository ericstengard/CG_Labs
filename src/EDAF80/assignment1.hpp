#pragma once

struct SpinConfiguration
{
	// TODO: Do we need to remove const?
	float inclination{0.0f}; // in rad
	float speed{0.0f};       // in rad/s
};

struct OrbitConfiguration
{
	float radius{0.0f};
	float inclination{0.0f}; // in rad
	float speed{0.0f};       // in rad/s
};
