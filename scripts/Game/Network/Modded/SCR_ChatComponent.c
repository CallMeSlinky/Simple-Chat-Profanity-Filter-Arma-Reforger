modded class SCR_ChatComponent
{	
	private ref map<int, int> playerWarnings = new map<int, int>;
	private int m_maxWarningsAllowed = 3;
	
	//-----------------------------------------------------------------------------
	
	override void OnNewMessage(string msg, int channel, int senderId)
	{
		Rpc(RpcAsk_MessageContainsProfanity, msg, channel, senderId);		
	}
 
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_MessageContainsProfanity(string message, int channel, int senderId)
	{	
		if (ContainsProfanity(message) && !GetGame().GetPlayerManager().HasPlayerRole(senderId, EPlayerRole.ADMINISTRATOR))
		{	
			AddUser(senderId);
			SetUsersWarningCount(senderId, GetUsersWarningCount(senderId) + 1);
			
			if (GetUsersWarningCount(senderId) == GetMaxWarningCount())
			{
			 	GetGame().GetPlayerManager().KickPlayer(senderId, SCR_PlayerManagerKickReason.KICKED_BY_GM);
				Print(string.Format("Kicked %1: Max Warning Threshold Hit", GetGame().GetPlayerManager().GetPlayerName(senderId)), LogLevel.NORMAL);
				return;
			}
			
			Rpc(NotifyPlayer, GetUsersWarningCount(senderId), GetMaxWarningCount());
			return;
		}
		Rpc(ExecuteMessage, message, channel, senderId);
	}
 
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast, RplCondition.OwnerOnly)]	
	protected void NotifyPlayer(int playerWarningCount, int maxWarningCount)
	{		
		Notify(string.Format("You used a forbidden word. You have %1 warnings, %2 until you get kicked. Behave yourself!", playerWarningCount, maxWarningCount - playerWarningCount));
	}
 
	protected void AddUser(int senderId)
	{	
		if (playerWarnings.Contains(senderId))
			return;
		playerWarnings.Insert(senderId, 0);
	}
	
	protected int GetUsersWarningCount(int senderId)
	{
		return playerWarnings.Get(senderId);
	}
	
	protected int GetMaxWarningCount()
	{
		return m_maxWarningsAllowed;
	}
 
	protected void SetUsersWarningCount(int senderId, int warningCount)
	{
		playerWarnings.Set(senderId, warningCount);
	}
	
	protected bool ContainsProfanity(string message)
	{
		array<string> words = new array<string>();
		message.ToLower();
		message.Split(" ", words, true);
		
		
		FileHandle file = FileIO.OpenFile("bad-words.txt", FileMode.READ);
		
		if (!file)
		{
			Print("Couldn't open file", LogLevel.ERROR);
			return false;
		}
		string temp;
		while (file.FGets(temp) > 0)
		{
			if (words.Contains(temp))
			{
				file.CloseFile();
				return true;
			}
		}
		file.CloseFile();
		return false;
	}
	
	protected void Notify(string message)
	{	
		SCR_HintManagerComponent hintsManager = SCR_HintManagerComponent.GetInstance();
		hintsManager.ShowCustom(message, "Chat");
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]	
	protected void ExecuteMessage(string msg, int channel, int senderId)
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		
		if (!mgr)
			return;
		
		mgr.OnNewMessageGeneral(msg, channel, senderId);
	}
};
