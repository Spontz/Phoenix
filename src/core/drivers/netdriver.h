// netdriver.h
// Spontz Demogroup

#pragma once

#include "core/drivers/net/dyad.h"

#include <vector>
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
	std::string processMessage(const std::string& pszMessage) const;
	void sendMessage(std::string const& sMessage) const;

public:
	int32_t m_iPortReceive;
	int32_t m_iPortSend;

private:
	// Dyad Callbacks
	static void dyadOnData(dyad_Event* const pDyadEvent);
	static void dyadOnAccept(dyad_Event* const pDyadEvent);
	static void dyadOnListen(dyad_Event* const pDyadEvent);
	static void dyadOnError(dyad_Event* const pDyadEvent);
	static void dyadOnConnect(dyad_Event* const pDyadEvent);

private:
	std::vector<std::string> splitMessage(const std::string& message) const;

private:
	bool m_bInitialized_;
	bool m_bConnectedToEditor_;
	dyad_Stream* m_pServConnect_;
};
