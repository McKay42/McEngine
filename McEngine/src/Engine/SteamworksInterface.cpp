//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		steamworks api wrapper
//
// $NoKeywords: $steam
//===============================================================================//

#include "SteamworksInterface.h"

#include "Engine.h"
#include "ConVar.h"
#include "Timer.h"
#include "Environment.h"

#ifdef MCENGINE_FEATURE_STEAMWORKS

#include "steam_api.h"

#endif

#define STEAMWORKS_APPID 0

SteamworksInterface *steam = NULL;

ConVar debug_steam("debug_steam", false, FCVAR_NONE);
ConVar steam_timeout("steam_timeout", 15.0f, FCVAR_NONE, "timeout in seconds for some steamworks API calls");

#ifdef MCENGINE_FEATURE_STEAMWORKS

UString SteamworksInterfaceAPICallFailureToString(ESteamAPICallFailure reason)
{
	switch (reason)
	{
	case k_ESteamAPICallFailureNone:				// no failure
		return "None";
	case k_ESteamAPICallFailureSteamGone:			// the local Steam process has gone away
		return "SteamGone";
	case k_ESteamAPICallFailureNetworkFailure:		// the network connection to Steam has been broken, or was already broken
		return "NetworkFailure";
	case k_ESteamAPICallFailureInvalidHandle:		// the SteamAPICall_t handle passed in no longer exists
		return "InvalidHandle";
	case k_ESteamAPICallFailureMismatchedCallback:	// GetAPICallResult() was called with the wrong callback type for this API call
		return "MismatchedCallback";
	}

	return "UnknownError";
}

bool SteamworksInterfaceWaitAPICallBlocking(SteamAPICall_t apiCall, void *pCallback, size_t cubCallback, int k_iCallback, bool ignoreTimeout = false)
{
	if (cubCallback < 1 || pCallback == NULL) return false;

	if (debug_steam.getBool())
		debugLog("SteamworksInterfaceWaitAPICallBlocking() ...\n");

	Timer t;
	t.start();

	bool timeout = false;
	bool completed = false;
	bool error = false;
	do
	{
		completed = SteamUtils()->IsAPICallCompleted(apiCall, &error);

		t.update();
		timeout = (t.getElapsedTime() > steam_timeout.getFloat()) && !ignoreTimeout;
	}
	while (!completed && !timeout);

	if (timeout || !completed)
	{
		debugLog("STEAM: API call timeout!\n");
		return false;
	}

	if (error)
	{
		const ESteamAPICallFailure reason = SteamUtils()->GetAPICallFailureReason(apiCall);
		const UString stringReason = SteamworksInterfaceAPICallFailureToString(reason);
		debugLog("STEAM: API call error %i (1) (%s)!\n", reason, stringReason.toUtf8());
	}

	if (debug_steam.getBool())
		debugLog("SteamworksInterfaceWaitAPICallBlocking() done.\n");

	if (completed)
	{
		if (SteamUtils()->GetAPICallResult(apiCall, pCallback, cubCallback, k_iCallback, &error))
		{
			if (error)
			{
				const ESteamAPICallFailure reason = SteamUtils()->GetAPICallFailureReason(apiCall);
				const UString stringReason = SteamworksInterfaceAPICallFailureToString(reason);
				debugLog("STEAM: API call error %i (2) (%s)!\n", reason, stringReason.toUtf8());
			}

			return true;
		}
	}

	return false;
}

#endif

SteamworksInterface::SteamworksInterface()
{
	steam = this;

	m_bReady = false;

	m_pendingItemUpdateHandle = 0;

	if (engine->getArgs().length() > 0 && engine->getArgs().find("nosteam") != -1) return;

#ifdef MCENGINE_FEATURE_STEAMWORKS

	m_pendingItemUpdateHandle = k_UGCUpdateHandleInvalid;

	// init and autorestart via steam if necessary
	// NOTE: this forces users to use steam, and if someone started without steam then it was probably intentional, therefore commented for now
	/*
	if (SteamAPI_RestartAppIfNecessary(STEAMWORKS_APPID))
	{
		debugLog("STEAM: RestartAppIfNecessary, shutting down ...\n");
		engine->shutdown();
		return;
	}
	*/

	if (!SteamAPI_Init())
	{
		debugLog("STEAM: SteamAPI_Init() failed!\n");
		return;
	}

	m_bReady = true;

	debugLog("STEAM: Logged in as \"%s\"\n", SteamFriends()->GetPersonaName());

#endif
}

SteamworksInterface::~SteamworksInterface()
{
	steam = NULL;

#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (m_bReady)
		SteamAPI_Shutdown();

#endif

	m_bReady = false;
}

void SteamworksInterface::update()
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;

	SteamAPI_RunCallbacks();

#endif
}

uint64_t SteamworksInterface::createWorkshopItem()
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return k_PublishedFileIdInvalid;

	const SteamAPICall_t apiCall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), EWorkshopFileType::k_EWorkshopFileTypeCommunity);

	CreateItemResult_t res;
	if (!SteamworksInterfaceWaitAPICallBlocking(apiCall, &res, sizeof(CreateItemResult_t), CreateItemResult_t::k_iCallback))
		return k_PublishedFileIdInvalid;

	if (res.m_eResult != EResult::k_EResultOK)
	{
		handleLastError(res.m_eResult);
		debugLog("STEAM: CreateItem() error %i (%s)!\n", res.m_eResult, m_sLastError.toUtf8());
		return k_PublishedFileIdInvalid;
	}

	if (res.m_bUserNeedsToAcceptWorkshopLegalAgreement)
		handleWorkshopLegalAgreementNotAccepted(res.m_nPublishedFileId);

	return res.m_nPublishedFileId;

#else

	return 0;

#endif
}

bool SteamworksInterface::pushWorkshopItemUpdate(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	m_pendingItemUpdateHandle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), publishedFileId);

	return (m_pendingItemUpdateHandle != k_UGCUpdateHandleInvalid);

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemTitle(UString title)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	return SteamUGC()->SetItemTitle(m_pendingItemUpdateHandle, title.toUtf8());

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemDescription(UString description)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	return SteamUGC()->SetItemDescription(m_pendingItemUpdateHandle, description.toUtf8());

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemLanguage(UString language)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	return SteamUGC()->SetItemUpdateLanguage(m_pendingItemUpdateHandle, language.toUtf8());

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemMetadata(UString metadata)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady || metadata.length() > k_cchDeveloperMetadataMax) return false;

	return SteamUGC()->SetItemMetadata(m_pendingItemUpdateHandle, metadata.toUtf8());

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemVisibility(bool visible, bool friendsOnly)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	return SteamUGC()->SetItemVisibility(m_pendingItemUpdateHandle, friendsOnly ? ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityFriendsOnly :
			(visible ? ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic : ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPrivate));

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemTags(std::vector<UString> tags)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;
	if (tags.size() < 1) return true;

	const char *ppStrings[tags.size()];

	for (size_t i=0; i<tags.size(); i++)
	{
		ppStrings[i] = tags[i].toUtf8();
	}

	SteamParamStringArray_t tagArray;
	tagArray.m_nNumStrings = tags.size();
	tagArray.m_ppStrings = ppStrings;

	return SteamUGC()->SetItemTags(m_pendingItemUpdateHandle, &tagArray);

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemContent(UString contentFolderAbsolutePath)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;
	if (contentFolderAbsolutePath.length() < 2) return false;

	return SteamUGC()->SetItemContent(m_pendingItemUpdateHandle, contentFolderAbsolutePath.toUtf8());

#else

	return false;

#endif
}

bool SteamworksInterface::setWorkshopItemPreview(UString previewFileAbsolutePath)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;
	if (previewFileAbsolutePath.length() < 2) return false;

	return SteamUGC()->SetItemPreview(m_pendingItemUpdateHandle, previewFileAbsolutePath.toUtf8());

#else

	return false;

#endif
}

bool SteamworksInterface::popWorkshopItemUpdate(UString changeNote)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	const SteamAPICall_t apiCall = SteamUGC()->SubmitItemUpdate(m_pendingItemUpdateHandle, (changeNote.length() > 0 ? changeNote.toUtf8() : NULL));

	SubmitItemUpdateResult_t res;
	if (!SteamworksInterfaceWaitAPICallBlocking(apiCall, &res, sizeof(SubmitItemUpdateResult_t), SubmitItemUpdateResult_t::k_iCallback, true))
		return false;

	// TODO: some upload progress would be nice
	/*
	uint64_t bytesProcessed = 0;
	uint64_t bytesTotal = 0;
	EItemUpdateStatus status = SteamUGC()->GetItemUpdateProgress(m_pendingItemUpdateHandle, &bytesProcessed, &bytesTotal);
	*/

	if (res.m_eResult != EResult::k_EResultOK)
	{
		handleLastError(res.m_eResult);
		debugLog("STEAM: SubmitItemUpdate() error %i (%s)!\n", res.m_eResult, m_sLastError.toUtf8());
		return false;
	}

	if (res.m_bUserNeedsToAcceptWorkshopLegalAgreement)
		handleWorkshopLegalAgreementNotAccepted(res.m_nPublishedFileId);

	m_pendingItemUpdateHandle = k_UGCUpdateHandleInvalid;

	return true;

#else

	return false;

#endif
}

std::vector<uint64_t> SteamworksInterface::getWorkshopSubscribedItems()
{
	std::vector<uint64_t> subscribedItems;

#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return subscribedItems;

	const uint32_t numSubscribedItems = SteamUGC()->GetNumSubscribedItems();

	if (numSubscribedItems > 0)
	{
		subscribedItems.resize(numSubscribedItems);

		const uint32_t numPopulatedSubscribedItems = SteamUGC()->GetSubscribedItems((PublishedFileId_t*)(&subscribedItems[0]), numSubscribedItems);

		if (numPopulatedSubscribedItems != numSubscribedItems)
		{
			debugLog("STEAM: GetSubscribedItems() warning, only got %u/%u!\n", numPopulatedSubscribedItems, numSubscribedItems);
			subscribedItems.resize(numPopulatedSubscribedItems);
		}
	}

#endif

	return subscribedItems;
}

bool SteamworksInterface::isWorkshopSubscribedItemInstalled(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	const uint32_t itemState = SteamUGC()->GetItemState(publishedFileId);

	if (debug_steam.getBool())
		debugLog("STEAM: SteamUGC()->GetItemState( %llu ) = %i\n", publishedFileId, itemState);

	return ((itemState & EItemState::k_EItemStateSubscribed) && (itemState & EItemState::k_EItemStateInstalled));

#endif

	return false;
}

bool SteamworksInterface::isWorkshopSubscribedItemDownloading(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	const uint32_t itemState = SteamUGC()->GetItemState(publishedFileId);

	if (debug_steam.getBool())
		debugLog("STEAM: SteamUGC()->GetItemState( %llu ) = %i\n", publishedFileId, itemState);

	return ((itemState & EItemState::k_EItemStateDownloading) || (itemState & EItemState::k_EItemStateDownloadPending));

#endif

	return false;
}

UString SteamworksInterface::getWorkshopItemInstallInfo(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return "";

	const uint32_t stringBufferSize = k_cchFilenameMax + 1;

	uint64 sizeOnDisk = 0; // NOTE: steam's uint64!
	uint32_t lastUpdated = 0;

	char stringBuffer[stringBufferSize];
	memset(stringBuffer, '\0', stringBufferSize);

	PublishedFileId_t publishedFileId_t = publishedFileId;

	if (SteamUGC()->GetItemInstallInfo(publishedFileId_t, &sizeOnDisk, stringBuffer, (stringBufferSize - 1), &lastUpdated))
		return UString(stringBuffer);

#endif

	return "";
}

std::vector<SteamworksInterface::WorkshopItemDetails> SteamworksInterface::getWorkshopItemDetails(const std::vector<uint64_t> &publishedFileIds)
{
	std::vector<WorkshopItemDetails> results;

#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady || publishedFileIds.size() < 1) return results;

	std::vector<PublishedFileId_t> publishedFileId_ts;
	publishedFileId_ts.reserve(publishedFileIds.size());
	for (size_t i=0; i<publishedFileIds.size(); i++)
	{
		publishedFileId_ts.push_back(publishedFileIds[i]);
	}

	const uint32_t numPagesToRequest = (publishedFileIds.size() / kNumUGCResultsPerPage) + 1;
	for (uint32_t p=0; p<numPagesToRequest; p++)
	{
		const uint32_t requestDataOffset = clamp<uint32_t>(p * kNumUGCResultsPerPage, 0, publishedFileIds.size());
		const uint32_t requestDataSize = clamp<uint32_t>(kNumUGCResultsPerPage, 0, publishedFileIds.size() - requestDataOffset);

		if (requestDataSize < 1)
			break;

		const UGCQueryHandle_t handle = SteamUGC()->CreateQueryUGCDetailsRequest(&publishedFileId_ts[requestDataOffset], requestDataSize);
		{
			if (handle != k_UGCQueryHandleInvalid)
			{
				const SteamAPICall_t apiCall = SteamUGC()->SendQueryUGCRequest(handle);

				SteamUGCQueryCompleted_t res;
				if (!SteamworksInterfaceWaitAPICallBlocking(apiCall, &res, sizeof(SteamUGCQueryCompleted_t), SteamUGCQueryCompleted_t::k_iCallback))
				{
					SteamUGC()->ReleaseQueryUGCRequest(handle);
					return results;
				}

				if (res.m_eResult != EResult::k_EResultOK)
				{
					handleLastError(res.m_eResult);
					debugLog("STEAM: SendQueryUGCRequest() error %i (%s)!\n", res.m_eResult, m_sLastError.toUtf8());
					SteamUGC()->ReleaseQueryUGCRequest(handle);
					return results;
				}

				for (uint32_t r=0; r<res.m_unNumResultsReturned; r++)
				{
					SteamUGCDetails_t details;
					if (SteamUGC()->GetQueryUGCResult(res.m_handle, r, &details))
					{
						WorkshopItemDetails ret;

						// id
						{
							ret.publishedFileId = details.m_nPublishedFileId;
						}

						// title
						{
							const int stringBufferSize = k_cchPublishedDocumentTitleMax + 1;
							char stringBuffer[stringBufferSize];
							memset(stringBuffer, '\0', stringBufferSize);
							memcpy(stringBuffer, details.m_rgchTitle, k_cchPublishedDocumentTitleMax);

							ret.title = UString(stringBuffer);
						}

						// description
						{
							const int stringBufferSize = k_cchPublishedDocumentDescriptionMax + 1;
							char stringBuffer[stringBufferSize];
							memset(stringBuffer, '\0', stringBufferSize);
							memcpy(stringBuffer, details.m_rgchDescription, k_cchPublishedDocumentDescriptionMax);

							ret.description = UString(stringBuffer);
						}

						results.push_back(ret);
					}
					else
					{
						debugLog("STEAM: GetQueryUGCResult() error (%u)!\n", r);
						break;
					}
				}
			}
			else
			{
				debugLog("STEAM: CreateQueryUGCDetailsRequest() error!\n");
				return results;
			}
		}
		SteamUGC()->ReleaseQueryUGCRequest(handle);
	}

#endif

	return results;
}

void SteamworksInterface::forceWorkshopItemUpdateDownload(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;

	// NOTE: ignoring waiting for DownloadItemResult_t, since this case is handled by isWorkshopSubscribedItemDownloading()
	SteamUGC()->DownloadItem(publishedFileId, true);

#endif
}

void SteamworksInterface::startWorkshopItemPlaytimeTracking(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;

	PublishedFileId_t publishedFileId_t = publishedFileId;

	SteamUGC()->StartPlaytimeTracking(&publishedFileId_t, 1);

#endif
}

void SteamworksInterface::stopWorkshopItemPlaytimeTracking(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;

	PublishedFileId_t publishedFileId_t = publishedFileId;

	SteamUGC()->StopPlaytimeTracking(&publishedFileId_t, 1);

#endif
}

void SteamworksInterface::stopWorkshopPlaytimeTrackingForAllItems()
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;

	SteamUGC()->StopPlaytimeTrackingForAllItems();

#endif
}

void SteamworksInterface::openURLInGameOverlay(UString url)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;

	SteamFriends()->ActivateGameOverlayToWebPage(url.toUtf8());

#endif
}

void SteamworksInterface::setRichPresence(UString key, UString value)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;
	if (key.length() < 1) return;

	SteamFriends()->SetRichPresence(key.toUtf8(), value.toUtf8());

#endif
}

bool SteamworksInterface::isGameOverlayEnabled()
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return false;

	return SteamUtils()->IsOverlayEnabled();

#endif

	return false;
}

UString SteamworksInterface::getUsername()
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return "";

	return UString(SteamFriends()->GetPersonaName());

#else

	return "";

#endif
}

void SteamworksInterface::handleWorkshopLegalAgreementNotAccepted(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;

	debugLog("STEAM: User needs to accept the workshop legal agreement first.\n");
	openWorkshopItemURLInGameOverlay(publishedFileId);

#endif
}

void SteamworksInterface::openWorkshopItemURLInGameOverlay(uint64_t publishedFileId)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	const UString url = UString::format("steam://url/CommunityFilePage/%llu", publishedFileId);

	if (!m_bReady)
	{
		env->openURLInDefaultBrowser(url);
		return;
	}

	if (isGameOverlayEnabled())
		openURLInGameOverlay(url);
	else
		env->openURLInDefaultBrowser(url);

#endif
}

void SteamworksInterface::handleLastError(int res)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	m_sLastError = "UnknownError";

	switch (res)
	{
	case EResult::k_EResultOK:					// success
		m_sLastError = "OK";
		break;
	case EResult::k_EResultFail:				// generic failure
		m_sLastError = "Fail";
		break;
	case EResult::k_EResultNoConnection:		// no/failed network connection
		m_sLastError = "NoConnection";
		break;
	case EResult::k_EResultInvalidPassword:		// password/ticket is invalid
		m_sLastError = "InvalidPassword";
		break;
	case EResult::k_EResultLoggedInElsewhere:	// same user logged in elsewhere
		m_sLastError = "LoggedInElsewhere";
		break;
	case EResult::k_EResultInvalidProtocolVer:	// protocol version is incorrect
		m_sLastError = "InvalidProtocolVer";
		break;
	case EResult::k_EResultInvalidParam:		// a parameter is incorrect
		m_sLastError = "InvalidParam";
		break;
	case EResult::k_EResultFileNotFound:		// file was not found
		m_sLastError = "FileNotFound";
		break;
	case EResult::k_EResultBusy:				// called method busy - action not taken
		m_sLastError = "Busy";
		break;
	case EResult::k_EResultInvalidState:		// called object was in an invalid state
		m_sLastError = "InvalidState";
		break;
	case EResult::k_EResultInvalidName:			// name is invalid
		m_sLastError = "InvalidName";
		break;
	case EResult::k_EResultInvalidEmail:		// email is invalid
		m_sLastError = "InvalidEmail";
		break;
	case EResult::k_EResultDuplicateName:		// name is not unique
		m_sLastError = "DuplicateName";
		break;
	case EResult::k_EResultAccessDenied:		// access is denied
		m_sLastError = "AccessDenied";
		break;
	case EResult::k_EResultTimeout:				// operation timed out
		m_sLastError = "Timeout";
		break;
	case EResult::k_EResultBanned:				// VAC2 banned
		m_sLastError = "Banned";
		break;
	case EResult::k_EResultAccountNotFound:		// account not found
		m_sLastError = "AccountNotFound";
		break;
	case EResult::k_EResultInvalidSteamID:		// steamID is invalid
		m_sLastError = "InvalidSteamID";
		break;
	case EResult::k_EResultServiceUnavailable:	// The requested service is currently unavailable
		m_sLastError = "ServiceUnavailable";
		break;
	case EResult::k_EResultNotLoggedOn:			// The user is not logged on
		m_sLastError = "NotLoggedOn";
		break;
	case EResult::k_EResultPending:				// Request is pending (may be in process, or waiting on third party)
		m_sLastError = "Pending";
		break;
	case EResult::k_EResultEncryptionFailure:	// Encryption or Decryption failed
		m_sLastError = "EncryptionFailure";
		break;
	case EResult::k_EResultInsufficientPrivilege:	// Insufficient privilege
		m_sLastError = "InsufficientPrivilege";
		break;
	case EResult::k_EResultLimitExceeded:		// Too much of a good thing
		m_sLastError = "LimitExceeded";
		break;
	case EResult::k_EResultRevoked:				// Access has been revoked (used for revoked guest passes)
		m_sLastError = "Revoked";
		break;
	case EResult::k_EResultExpired:				// License/Guest pass the user is trying to access is expired
		m_sLastError = "Expired";
		break;
	case EResult::k_EResultAlreadyRedeemed:		// Guest pass has already been redeemed by account, cannot be acked again
		m_sLastError = "AlreadyRedeemed";
		break;
	case EResult::k_EResultDuplicateRequest:	// The request is a duplicate and the action has already occurred in the past, ignored this time
		m_sLastError = "DuplicateRequest";
		break;
	case EResult::k_EResultAlreadyOwned:		// All the games in this guest pass redemption request are already owned by the user
		m_sLastError = "AlreadyOwned";
		break;
	case EResult::k_EResultIPNotFound:			// IP address not found
		m_sLastError = "IPNotFound";
		break;
	case EResult::k_EResultPersistFailed:		// failed to write change to the data store
		m_sLastError = "PersistFailed";
		break;
	case EResult::k_EResultLockingFailed:		// failed to acquire access lock for this operation
		m_sLastError = "LockingFailed";
		break;
	case EResult::k_EResultLogonSessionReplaced:
		m_sLastError = "LogonSessionReplaced";
		break;
	case EResult::k_EResultConnectFailed:
		m_sLastError = "ConnectFailed";
		break;
	case EResult::k_EResultHandshakeFailed:
		m_sLastError = "HandshakeFailed";
		break;
	case EResult::k_EResultIOFailure:
		m_sLastError = "IOFailure";
		break;
	case EResult::k_EResultRemoteDisconnect:
		m_sLastError = "RemoteDisconnect";
		break;
	case EResult::k_EResultShoppingCartNotFound:// failed to find the shopping cart requested
		m_sLastError = "ShoppingCartNotFound";
		break;
	case EResult::k_EResultBlocked:				// a user didn't allow it
		m_sLastError = "Blocked";
		break;
	case EResult::k_EResultIgnored:				// target is ignoring sender
		m_sLastError = "Ignored";
		break;
	case EResult::k_EResultNoMatch:				// nothing matching the request found
		m_sLastError = "NoMatch";
		break;
	case EResult::k_EResultAccountDisabled:
		m_sLastError = "AccountDisabled";
		break;
	case EResult::k_EResultServiceReadOnly:		// this service is not accepting content changes right now
		m_sLastError = "ServiceReadOnly";
		break;
	case EResult::k_EResultAccountNotFeatured:	// account doesn't have value, so this feature isn't available
		m_sLastError = "AccountNotFeatured";
		break;
	case EResult::k_EResultAdministratorOK:		// allowed to take this action, but only because requester is admin
		m_sLastError = "AdministratorOK";
		break;
	case EResult::k_EResultContentVersion:		// A Version mismatch in content transmitted within the Steam protocol.
		m_sLastError = "ContentVersion";
		break;
	case EResult::k_EResultTryAnotherCM:		// The current CM can't service the user making a request, user should try another.
		m_sLastError = "TryAnotherCM";
		break;
	case EResult::k_EResultPasswordRequiredToKickSession:	// You are already logged in elsewhere, this cached credential login has failed.
		m_sLastError = "PasswordRequiredToKickSession";
		break;
	case EResult::k_EResultAlreadyLoggedInElsewhere:		// You are already logged in elsewhere, you must wait
		m_sLastError = "AlreadyLoggedInElsewhere";
		break;
	case EResult::k_EResultSuspended:			// Long running operation (content download) suspended/paused
		m_sLastError = "Suspended";
		break;
	case EResult::k_EResultCancelled:			// Operation canceled (typically by user: content download)
		m_sLastError = "Cancelled";
		break;
	case EResult::k_EResultDataCorruption:		// Operation canceled because data is ill formed or unrecoverable
		m_sLastError = "DataCorruption";
		break;
	case EResult::k_EResultDiskFull:			// Operation canceled - not enough disk space.
		m_sLastError = "DiskFull";
		break;
	case EResult::k_EResultRemoteCallFailed:	// an remote call or IPC call failed
		m_sLastError = "RemoteCallFailed";
		break;
	case EResult::k_EResultPasswordUnset:		// Password could not be verified as it's unset server side
		m_sLastError = "PasswordUnset";
		break;
	case EResult::k_EResultExternalAccountUnlinked:			// External account (PSN, Facebook...) is not linked to a Steam account
		m_sLastError = "ExternalAccountUnlinked";
		break;
	case EResult::k_EResultPSNTicketInvalid:				// PSN ticket was invalid
		m_sLastError = "PSNTicketInvalid";
		break;
	case EResult::k_EResultExternalAccountAlreadyLinked:	// External account (PSN, Facebook...) is already linked to some other account, must explicitly request to replace/delete the link first
		m_sLastError = "ExternalAccountAlreadyLinked";
		break;
	case EResult::k_EResultRemoteFileConflict:			// The sync cannot resume due to a conflict between the local and remote files
		m_sLastError = "RemoteFileConflict";
		break;
	case EResult::k_EResultIllegalPassword:				// The requested new password is not legal
		m_sLastError = "IllegalPassword";
		break;
	case EResult::k_EResultSameAsPreviousValue:			// new value is the same as the old one ( secret question and answer )
		m_sLastError = "SameAsPreviousValue";
		break;
	case EResult::k_EResultAccountLogonDenied:			// account login denied due to 2nd factor authentication failure
		m_sLastError = "AccountLogonDenied";
		break;
	case EResult::k_EResultCannotUseOldPassword:		// The requested new password is not legal
		m_sLastError = "CannotUseOldPassword";
		break;
	case EResult::k_EResultInvalidLoginAuthCode:		// account login denied due to auth code invalid
		m_sLastError = "InvalidLoginAuthCode";
		break;
	case EResult::k_EResultAccountLogonDeniedNoMail:	// account login denied due to 2nd factor auth failure - and no mail has been sent
		m_sLastError = "AccountLogonDeniedNoMail";
		break;
	case EResult::k_EResultHardwareNotCapableOfIPT:
		m_sLastError = "HardwareNotCapableOfIPT";
		break;
	case EResult::k_EResultIPTInitError:
		m_sLastError = "IPTInitError";
		break;
	case EResult::k_EResultParentalControlRestricted:	// operation failed due to parental control restrictions for current user
		m_sLastError = "ParentalControlRestricted";
		break;
	case EResult::k_EResultFacebookQueryError:			// Facebook query returned an error
		m_sLastError = "FacebookQueryError";
		break;
	case EResult::k_EResultExpiredLoginAuthCode:		// account login denied due to auth code expired
		m_sLastError = "ExpiredLoginAuthCode";
		break;
	case EResult::k_EResultIPLoginRestrictionFailed:
		m_sLastError = "IPLoginRestrictionFailed";
		break;
	case EResult::k_EResultAccountLockedDown:
		m_sLastError = "AccountLockedDown";
		break;
	case EResult::k_EResultAccountLogonDeniedVerifiedEmailRequired:
		m_sLastError = "AccountLogonDeniedVerifiedEmailRequired";
		break;
	case EResult::k_EResultNoMatchingURL:
		m_sLastError = "NoMatchingURL";
		break;
	case EResult::k_EResultBadResponse:				// parse failure, missing field, etc.
		m_sLastError = "BadResponse";
		break;
	case EResult::k_EResultRequirePasswordReEntry:	// The user cannot complete the action until they re-enter their password
		m_sLastError = "RequirePasswordReEntry";
		break;
	case EResult::k_EResultValueOutOfRange:			// the value entered is outside the acceptable range
		m_sLastError = "ValueOutOfRange";
		break;
	case EResult::k_EResultUnexpectedError:			// something happened that we didn't expect to ever happen
		m_sLastError = "UnexpectedError";
		break;
	case EResult::k_EResultDisabled:				// The requested service has been configured to be unavailable
		m_sLastError = "Disabled";
		break;
	case EResult::k_EResultInvalidCEGSubmission:	// The set of files submitted to the CEG server are not valid !
		m_sLastError = "InvalidCEGSubmission";
		break;
	case EResult::k_EResultRestrictedDevice:		// The device being used is not allowed to perform this action
		m_sLastError = "RestrictedDevice";
		break;
	case EResult::k_EResultRegionLocked:			// The action could not be complete because it is region restricted
		m_sLastError = "RegionLocked";
		break;
	case EResult::k_EResultRateLimitExceeded:		// Temporary rate limit exceeded, try again later, different from case EResult::k_EResultLimitExceeded which may be permanent
		m_sLastError = "RateLimitExceeded";
		break;
	case EResult::k_EResultAccountLoginDeniedNeedTwoFactor:		// Need two-factor code to login
		m_sLastError = "AccountLoginDeniedNeedTwoFactor";
		break;
	case EResult::k_EResultItemDeleted:					// The thing we're trying to access has been deleted
		m_sLastError = "ItemDeleted";
		break;
	case EResult::k_EResultAccountLoginDeniedThrottle:	// login attempt failed, try to throttle response to possible attacker
		m_sLastError = "AccountLoginDeniedThrottle";
		break;
	case EResult::k_EResultTwoFactorCodeMismatch:		// two factor code mismatch
		m_sLastError = "TwoFactorCodeMismatch";
		break;
	case EResult::k_EResultTwoFactorActivationCodeMismatch:	// activation code for two-factor didn't match
		m_sLastError = "TwoFactorActivationCodeMismatch";
		break;
	case EResult::k_EResultAccountAssociatedToMultiplePartners:	// account has been associated with multiple partners
		m_sLastError = "AccountAssociatedToMultiplePartners";
		break;
	case EResult::k_EResultNotModified:						// data not modified
		m_sLastError = "NotModified";
		break;
	case EResult::k_EResultNoMobileDevice:					// the account does not have a mobile device associated with it
		m_sLastError = "NoMobileDevice";
		break;
	case EResult::k_EResultTimeNotSynced:					// the time presented is out of range or tolerance
		m_sLastError = "TimeNotSynced";
		break;
	case EResult::k_EResultSmsCodeFailed:					// SMS code failure (no match, none pending, etc.)
		m_sLastError = "SmsCodeFailed";
		break;
	case EResult::k_EResultAccountLimitExceeded:			// Too many accounts access this resource
		m_sLastError = "AccountLimitExceeded";
		break;
	case EResult::k_EResultAccountActivityLimitExceeded:	// Too many changes to this account
		m_sLastError = "AccountActivityLimitExceeded";
		break;
	case EResult::k_EResultPhoneActivityLimitExceeded:		// Too many changes to this phone
		m_sLastError = "PhoneActivityLimitExceeded";
		break;
	case EResult::k_EResultRefundToWallet:					// Cannot refund to payment method, must use wallet
		m_sLastError = "RefundToWallet";
		break;
	case EResult::k_EResultEmailSendFailure:		// Cannot send an email
		m_sLastError = "EmailSendFailure";
		break;
	case EResult::k_EResultNotSettled:				// Can't perform operation till payment has settled
		m_sLastError = "NotSettled";
		break;
	case EResult::k_EResultNeedCaptcha:				// Needs to provide a valid captcha
		m_sLastError = "NeedCaptcha";
		break;
	case EResult::k_EResultGSLTDenied:				// a game server login token owned by this token's owner has been banned
		m_sLastError = "GSLTDenied";
		break;
	case EResult::k_EResultGSOwnerDenied:			// game server owner is denied for other reason (account lock, community ban, vac ban, missing phone)
		m_sLastError = "GSOwnerDenied";
		break;
	case EResult::k_EResultInvalidItemType:			// the type of thing we were requested to act on is invalid
		m_sLastError = "InvalidItemType";
		break;
	case EResult::k_EResultIPBanned:				// the ip address has been banned from taking this action
		m_sLastError = "IPBanned";
		break;
	case EResult::k_EResultGSLTExpired:				// this token has expired from disuse; can be reset for use
		m_sLastError = "GSLTExpired";
		break;
	case EResult::k_EResultInsufficientFunds:		// user doesn't have enough wallet funds to complete the action
		m_sLastError = "InsufficientFunds";
		break;
	case EResult::k_EResultTooManyPending:			// There are too many of this thing pending already
		m_sLastError = "TooManyPending";
		break;
	case EResult::k_EResultNoSiteLicensesFound:		// No site licenses found
		m_sLastError = "NoSiteLicensesFound";
		break;
	case EResult::k_EResultWGNetworkSendExceeded:	// the WG couldn't send a response because we exceeded max network send size
		m_sLastError = "WGNetworkSendExceeded";
		break;
	case EResult::k_EResultAccountNotFriends:		// the user is not mutually friends
		m_sLastError = "AccountNotFriends";
		break;
	case EResult::k_EResultLimitedUserAccount:		// the user is limited
		m_sLastError = "LimitedUserAccount";
		break;
	}

#endif
}
