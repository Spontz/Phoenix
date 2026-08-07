// MathDriver.h
// Spontz Demogroup

#pragma once

namespace Phoenix {

	class Model;

	class MathDriver final {

	public:
		MathDriver(Section* pSection);

	public:
		bool isValid() const { return valid; }
		bool compileFormula();
		void executeFormula();

		// Publishes the camera resolved by Model::PreCalc() into the "modelcam_*" expression
		// variables. Sections call this between precalculation and expression evaluation, so a
		// formula can react to the model camera within the same frame.
		void setModelCamera(const Model& model);

	public:
		std::string					expression;
		exprtk::symbol_table<float>	SymbolTable;
		exprtk::expression<float>	Expression;
		exprtk::parser<float>		Parser;

	private:
		// Storage backing the "modelcam_*" variables. exprtk binds these by reference when the
		// symbol table is built, so updating them is enough to change the next evaluation.
		struct ModelCameraVars final {
			float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
			float frontX = 0.0f, frontY = 0.0f, frontZ = -1.0f;
			float upX = 0.0f, upY = 1.0f, upZ = 0.0f;
			float yaw = 0.0f, pitch = 0.0f, roll = 0.0f, fov = 0.0f;
		};

		ModelCameraVars				m_modelCamVars;
		bool						valid; // Is the expression valid?
	};
}