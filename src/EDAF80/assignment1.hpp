#pragma once

struct SpinConfiguration
{
	// TODO: Do we need to remove const?
	float inclination{0.0f}; // in rad
	float speed{0.0f};       // in rad/s
};

struct OrbitConfiguration
{
	float const radius{0.0f};
	float const inclination{0.0f}; // in rad
	float const speed{0.0f};       // in rad/s
};
