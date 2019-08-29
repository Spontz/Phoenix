#include "main.h"

#include "core/section.h"

#include "sph.hpp"

namespace sections {

	float smoothing_kernel_double_cos(glm::vec3 const& p0, glm::vec3 const& p1) {
		constexpr float h = 1.0f;
		constexpr float k = 1.0f;
		constexpr float pi = glm::pi<float>();
		constexpr float a3 = pi / ((4.0f * pi * pi - 30.0f) * (k * h) * (k * h) * (k * h));

		const float r = glm::length(p0 - p1);
		const float s = r / h;

		if (k < s)
			return 0.0f;

		return 4.0f * glm::cos(glm::pi<float>() / k * s) + glm::cos(2.0f * glm::pi<float>() / k * s) + 3.0f;
	}

	bool SPH::load() {
		auto& demo_kernel = demokernel::GetInstance();
		const std::string uri = demo_kernel.dataFolder + "/resources/shaders/sph/billboard.";
		pass_sph_billboard_ = demo_kernel.shaderManager.addShader(uri + "vs", uri + "ps", uri + "gs");
		return true;
	}

	void SPH::init() {
		// hack: allow to provide initial state
		particles_.clear();
		for (int x = -100; x <= 100; ++x)
			for (int y = -100; y <= 100; ++y)
				for (int z = -100; z <= 100; ++z)
					particles_.push_back({ v3(x, y, z) });
	}

	void SPH::exec() {
		Simulate();
		Draw();
	}

	void SPH::Draw() {

	}


	void SPH::Simulate() {
		real elapsed = real(1) / real(kSimFreqHz); // hack: get from engine

		grid_.clear();

		std::set<std::tuple<int, int, int>> visited;
		for (size_t i = 0; i < particles_pos_.size(); ++i) {
			auto const& p = particles_pos_[i];

			auto const grid_idx = t_grid_index{
				static_cast<int>(p.x),
				static_cast<int>(p.y),
				static_cast<int>(p.z)
			};

			grid_[grid_idx].push_back(i);
			visited.insert(grid_idx);
		}

		auto couple = [&](size_t a_idx, size_t b_idx) {
			auto const& a_pos = particles_pos_[b_idx];
			auto const& b_pos = particles_pos_[a_idx];
			auto& dst = particles_[a_idx];

			const real f = smoothing_kernel_double_cos(a_pos, b_pos);
			dst.f += glm::normalize(a_pos - b_pos) * f;
		};

		// hack: asuming particle mass == 1
		static constexpr real kMass = real(1);
		static constexpr real kFriction = real(.999);

		for (const auto& i : visited) {
			auto const x = std::get<0>(i);
			auto const y = std::get<1>(i);
			auto const z = std::get<2>(i);

			auto const idx = std::tuple<int, int, int>{ x,y,z };

			for (auto const idx2 : grid_[idx]) {
				auto& particle = particles_[idx2];
				auto& particle_pos = particles_pos_[idx2];

				// update forces
				particle.f = v3(0, 0, 0);
				for (auto const idx3 : grid_[idx]) {
					if (idx2 == idx3)
						continue;
					couple(idx2, idx3);

					for (auto i_x = -1; i_x < 1; i_x += 2)
						for (auto i_y = -1; i_y < 1; i_y += 2)
							for (auto i_z = -1; i_z < 1; i_z += 2)
								for (auto& idx3 : grid_[std::tuple<int, int, int>{ x + i_x, y + i_y, z + i_z }])
									couple(idx2, idx3);
				}

				// update positions
				// hack: asuming particle mass == 1
				particle.a = particle.f / kMass;
				particle.v += particle.a / kSimFreqHz;
				particle.v *= kFriction;
				particle_pos += particle.v / kSimFreqHz;
			}
		}
	}

	void end() {

	}


}
