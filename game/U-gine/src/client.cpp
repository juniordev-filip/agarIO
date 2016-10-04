//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#pragma warning(disable: 4100)
#pragma warning(disable: 4710)
#pragma warning(disable: 4820)
#pragma warning(disable: 4996)

#include "../include/u-gine.h"
#include "ClientENet.h"
#include "PacketENet.h"
#include "Random.h"
#include "Buffer.h"
#include "Serializable.h"
#include "pthread.h"
//AgarIO_common
#include "entity.h"
#include "defs.h"
#include "serialize.h"

ENet::CClienteENet *pClient;
pthread_mutex_t gMutexClient = PTHREAD_MUTEX_INITIALIZER;

std::vector<ENet::CPacketENet *> gIncomingPackets;
pthread_mutex_t gMutexPackets = PTHREAD_MUTEX_INITIALIZER;

ENet::CPeerENet * gPeer;

std::vector<aioc::Entity *> gEntities;

void DrawEntities() {
	std::vector<aioc::Entity *>::iterator entIt = gEntities.begin();
	while (entIt != gEntities.end()) {
		Renderer::Instance().DrawEllipse((*entIt)->GetX(), (*entIt)->GetY(),
			(*entIt)->GetRadius(), (*entIt)->GetRadius());
		++entIt;
	}
}

void ProcessServerCommand(enet_uint8 command, CBuffer &data) {
	switch (command) {
		case C_PLAYER_CONNECTED:

			break;
		case C_INITIAL_PICKABLES:

			break;
		case C_SPAWN_PICKABLES:

			break;
		case C_PLAYERS_SNAPSHOT:
			//read data buffer and update all players
			break;
	}
}

void * UpdaterThread(void *) {
	enet_uint16 intToSend;
	CBuffer buffer, inBuffer, outBuffer;
	CRandom randInt;
	std::vector<ENet::CPacketENet *>::iterator delItr;
	std::vector<ENet::CPacketENet *>::iterator itr;
	enet_uint8 command;
	while (1) {
		/*pthread_mutex_lock(&gMutexPackets);
		pClient->Service(gIncomingPackets, 0);
		pthread_mutex_unlock(&gMutexPackets);*/
		pthread_mutex_lock(&gMutexPackets);
		pClient->Service(gIncomingPackets, 0);
		delItr = gIncomingPackets.begin();
		itr = gIncomingPackets.begin();
		while (itr != gIncomingPackets.end()) {
			if ((*itr)->GetType() == ENet::EPacketType::DATA) {
				printf_s("Received a packet!!!!!!!!!!!!!\n");
				inBuffer.Write((*itr)->GetData(), (*itr)->GetDataLength());
				aioc::DeserializeCommand(outBuffer, inBuffer, nullptr, command);
				ProcessServerCommand(command, outBuffer);
				inBuffer.Clear();
				delete *itr;
				itr = gIncomingPackets.erase(itr);
			}
		}
		gIncomingPackets.clear();
		pthread_mutex_unlock(&gMutexPackets);

		//intToSend = static_cast<enet_uint16>(randInt.GetRandUnsigned(0, 4000));
		intToSend = static_cast<enet_uint16>(1001);
		buffer.Clear();
		buffer.Write(&intToSend, sizeof(intToSend));
		pthread_mutex_lock(&gMutexClient);
		pClient->SendData(gPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);
		pthread_mutex_unlock(&gMutexClient);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	//U-gine conf
	Screen::Instance().Open(800, 600, false);
	Renderer::Instance().SetBlendMode(Renderer::BlendMode::SOLID);

	pClient = new ENet::CClienteENet();
	pClient->Init();

	gPeer = pClient->Connect("127.0.0.1", 1234, 2);

	/*aioc::Entity * entity = new aioc::Entity(static_cast<enet_uint16>(50),
		static_cast<enet_uint16>(50), static_cast<enet_uint16>(16));
	gEntities.push_back(entity);*/


	enet_uint16 intToSend;
	CBuffer buffer, inBuffer, outBuffer;
	CRandom randInt;

	pthread_t tUpdater;
	pthread_create(&tUpdater, nullptr, UpdaterThread, nullptr);

	std::vector<ENet::CPacketENet *>::iterator delItr;
	std::vector<ENet::CPacketENet *>::iterator itr;
	enet_uint8 command;
	while (Screen::Instance().IsOpened() && !Screen::Instance().KeyPressed(GLFW_KEY_ESC)) {
		Renderer::Instance().Clear();

		/*pthread_mutex_lock(&gMutexPackets);
		delItr = gIncomingPackets.begin();
		itr = gIncomingPackets.begin();
		while (itr != gIncomingPackets.end()) {
			if ((*itr)->GetType() == ENet::EPacketType::DATA) {
				printf_s("Received a packet!!!!!!!!!!!!!\n");
				inBuffer.Write((*itr)->GetData(), (*itr)->GetDataLength());
				aioc::DeserializeCommand(outBuffer, inBuffer, nullptr, command);
				inBuffer.Clear();
				delete *itr;
				itr = gIncomingPackets.erase(itr);
			}
		}
		pthread_mutex_unlock(&gMutexPackets);

		intToSend = static_cast<enet_uint16>(randInt.GetRandUnsigned(0, 4000));
		//intToSend = static_cast<enet_uint16>(1001);
		buffer.Clear();
		buffer.Write(&intToSend, sizeof(intToSend));
		pthread_mutex_lock(&gMutexClient);
		pClient->SendData(gPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);
		pthread_mutex_lock(&gMutexClient);*/

		DrawEntities();

		Screen::Instance().Refresh();
	}
	pthread_mutex_lock(&gMutexClient);
	pClient->Disconnect(gPeer);
	pthread_mutex_unlock(&gMutexClient);

	ResourceManager::Instance().FreeResources();

	return 0;
}