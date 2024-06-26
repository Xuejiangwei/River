﻿#include "RiverPch.h"
#include "KnowledgeNeuronApplication.h"
#include "ThirdLib/Haze/include/Haze.h"

KnowledgeNeuronApplication::KnowledgeNeuronApplication()
	: Application({ 1600, 900, 0, 0 })
{
}

KnowledgeNeuronApplication::~KnowledgeNeuronApplication()
{
}

void KnowledgeNeuronApplication::Initialize()
{
	m_HazeVM->CallFunction(HAZE_TEXT("初始化"));
}

Application* CreateApplication()
{
	return new KnowledgeNeuronApplication();
}