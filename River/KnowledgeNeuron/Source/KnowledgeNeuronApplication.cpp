#include "RiverPch.h"
#include "KnowledgeNeuronApplication.h"

KnowledgeNeuronApplication::KnowledgeNeuronApplication()
	: Application({ 1600, 900, 0, 0 })
	//: Application({ 480, 480, 0, 0 })
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