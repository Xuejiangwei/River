#include "RiverPch.h"
#include "KnowledgeNeuronApplication.h"

KnowledgeNeuronApplication::KnowledgeNeuronApplication()
	: Application()
{
}

KnowledgeNeuronApplication::~KnowledgeNeuronApplication()
{
}

void KnowledgeNeuronApplication::Initialize()
{
	LOG("Hello World");
}

Application* CreateApplication()
{
	return new KnowledgeNeuronApplication();
}