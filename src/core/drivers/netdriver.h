// netdriver.h
// Spontz Demogroup

#pragma once

#include "core/drivers/net/dyad.h"

#include <string>

class netDriver final {
public:
	static netDriver& GetInstance();

public:
	netDriver();
	~netDriver();

public:
	void init();
	void connectToEditor();
	void update() const;
	const char* getVersion() const;
	char* processMessage(const char* pszMessage) const;
	void sendMessage(std::string const& sMessage) const;

public:
	int32_t m_iPortReceive;
	int32_t m_iPortSend_;

private:
	// Dyad Callbacks
	static void dyadOnData(dyad_Event* const pDyadEvent);
	static void dyadOnAccept(dyad_Event* const pDyadEvent);
	static void dyadOnListen(dyad_Event* const pDyadEvent);
	static void dyadOnError(dyad_Event* const pDyadEvent);
	static void dyadOnConnect(dyad_Event* const pDyadEvent);

private:
	// Returns the requested parameter from the passed message (first parameter is 1) as a string
	std::string getParamString(const char* pszMessage, int32_t iRequestedParameter) const;

	// Returns the requested parameter from the passed message (first parameter is 1) as a floating point number
	float getParamFloat(const char* pszMessage, int32_t iRequestedParameter) const;

private:
	bool m_bInitialized_;
	bool m_bConnectedToEditor_;
	dyad_Stream* m_pServConnect_;
};
