#include "main.h"

#include "core/section.h"

#include "sph.h"

// todo: billboards should be big enough to raycast a sphere inside

namespace sections {

	// hack: get from cam
	const float kNearPlane = 1.0f;
	const float kFarPlane = 10000.0f;

	float smoothing_kernel_double_cos(glm::vec3 const& p0, glm::vec3 const& p1) {
		constexpr float h = 1.0f;
		constexpr float k = 1.0f;
		constexpr float pi = glm::pi<float>();
		constexpr float a3 = pi / ((4.0f * pi * pi - 30.0f) * (k * h) * (k * h) * (k * h));

		const float r = glm::length(p0 - p1);
		const float s = r / h;

		if (k < s)
			return 0.0f;

		return 4.0f * glm::cos(pi / k * s) + glm::cos(2.0f * pi / k * s) + 3.0f;
	}

	bool SPH::load() {
		auto& demo_kernel = demokernel::GetInstance();
		auto& shader_manager = demo_kernel.shaderManager;

		{
			// todo: shared route, too generic to be on a folder
			const std::string uri(demo_kernel.dataFolder + "/resources/shaders/sph/billboard.");
			billboard_shader_id_ = shader_manager.addShader(
				uri + "vs",
				uri + "ps",
				uri + "gs"
			);

			if (billboard_shader_id_ < 0)
				return false;

			p_billboard_shader_ = shader_manager.shader[billboard_shader_id_];
		}


		return true;
	}

	void SPH::init() {
		// hack: allow to provide initial state
		particles_.clear();
		for (int x = -100; x <= 100; ++x)
			for (int y = -100; y <= 100; ++y)
				for (int z = -100; z <= 100; ++z)
					particles_.push_back({ v3(x, y, z) });


		glGenBuffers(1, &gl_buffer_id_);
		glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_id_);
		glBufferData(
			GL_ARRAY_BUFFER,
			particles_.size() * 3 * sizeof(GLfloat),
			&particles_[0],
			GL_STREAM_DRAW
		);

	}

	void SPH::exec() {
		Simulate(); // todo: vectorize simulation, keep consistent freq.
		Draw();
	}

	void SPH::Draw() {
		auto& demo = demokernel::GetInstance();
		auto& gldrv = glDriver::GetInstance();

		p_billboard_shader_->use();

		const glm::mat4 proj = glm::perspective(
			glm::radians(demo.camera->Zoom), // hack: rename to fov_y, use radians in cams
			gldrv.GetFramebufferViewport().GetAspectRatio(),
			0.1f, // hack: get from cam
			10000.0f // hack: get from cam
		);

		const glm::mat4 view = demo.camera->GetViewMatrix();
		// const glm::mat4 proj = demo.camera->GetProjection(); // todo

		p_billboard_shader_->setValue("projection", proj);
		p_billboard_shader_->setValue("view", view);

		glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_id_);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(particles_.size()));
		// glDisableClientState(GL_VERTEX_ARRAY);
	}


	void SPH::Simulate() {
		real elapsed = real(1) / real(kSimFreqHz); // hack: get from engine

		grid_.clear();

		typedef std::tuple<int, int, int> t_grid_idx;

		std::set<t_grid_idx> visited;
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

		assert(glMapBuffer(gl_buffer_id_, GL_READ_WRITE) == &particles_pos_[0]);

		for (const auto& i : visited) {
			auto const x = std::get<0>(i);
			auto const y = std::get<1>(i);
			auto const z = std::get<2>(i);

			const t_grid_idx idx{ x, y, z };

			for (auto const idx2 : grid_[idx]) {
				auto& particle = particles_[idx2];
				auto& particle_pos = particles_pos_[idx2];

				// update forces
				particle.f = v3(0, 0, 0);
				for (auto const idx3 : grid_[idx]) {
					for (auto i_x = -1; i_x < 1; i_x += 2)
						for (auto i_y = -1; i_y < 1; i_y += 2)
							for (auto i_z = -1; i_z < 1; i_z += 2) {
								auto& g = grid_[
									t_grid_idx{ x + i_x, y + i_y, z + i_z }
								];
								for (auto& idx4 : g) {
									if (idx2 == idx4)
										continue;
									couple(idx2, idx4);
								}

							}
				}

				// update positions
				// hack: asuming particle mass == 1
				particle.a = particle.f / kMass;
				particle.v += particle.a / kSimFreqHz;
				particle.v *= kFriction;
				particle_pos += particle.v / kSimFreqHz;
			}
		}

		assert(glUnmapBuffer(gl_buffer_id_));
	}

	void end() {

	}


}
