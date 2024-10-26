// ModelManager.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Model.h"

namespace Phoenix {

	class ModelManager final {
		friend ImGuiLayer;
	public:
		SP_Model	addModel(std::string_view path);
		void		clear();

	private:
		std::vector<SP_Model> model;	// Model list
	};
}
