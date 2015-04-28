#include "stdafx.h"
#include "Player.h"
#include "PlayerManager.h"

PlayerManager* GPlayerManager = nullptr;

PlayerManager::PlayerManager() : mLock(LO_ECONOMLY_CLASS), mCurrentIssueId(0)
{

}

int PlayerManager::RegisterPlayer(std::shared_ptr<Player> player)
{
	FastSpinlockGuard exclusive(mLock);

	mPlayerMap[++mCurrentIssueId] = player;

	return mCurrentIssueId;
}

void PlayerManager::UnregisterPlayer(int playerId)
{
	FastSpinlockGuard exclusive(mLock);

	mPlayerMap.erase(playerId);
}


int PlayerManager::GetCurrentPlayers(PlayerList& outList)
{
	//TODO: mLock�� read���� �����ؼ� outList�� ���� �÷��̾���� ������ ��� total���� ���� �÷��̾��� �� ���� ��ȯ.
    FastSpinlockGuard read(mLock, false);

    for (auto& playr : mPlayerMap)
    {
        outList.push_back(playr.second);
    }

	int total = outList.size();

	return total;
}