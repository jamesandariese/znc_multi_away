#include <znc/IRCNetwork.h>
#include <znc/User.h>
#include <stdio.h>
#include <znc/Modules.h>

using std::vector;

class CMultiAwayMod : public CModule {
private:
  bool m_isAway; // If we've set ourselves as away, mark it here.
  bool m_forced; // If the user has forced us to be away, mark it here.
public:
  MODCONSTRUCTOR(CMultiAwayMod) {
    m_isAway = false;
    m_forced = false;
  }
  virtual void OnClientLogin() {
    DecideAwayness();    
  }

  virtual void OnClientDisconnect() {
    DecideAwayness();
  }

  void DecideAwayness() {
    int notAways = 0;
    int clients = 0;

    vector<CClient*> vUserClients = m_pUser->GetAllClients();
    for (size_t c = 0; c < vUserClients.size(); ++c) {
      CClient* pUserClient = vUserClients[c];
      
      if (!pUserClient->IsAway()) {
	++notAways;
      }
      ++clients;
    }

    char tmpchr[256];
    tmpchr[255] = '\0';
    snprintf(tmpchr, 255, "Clients NOT away: %d/%d", notAways, clients);
    PutModule(tmpchr);
    
    if (clients == 0) {
      if (!m_isAway) {
	m_isAway = true;
	PutIRC("AWAY :Detached");
      }
      return;
    } else if (notAways == 0) {
      if (!m_isAway) {
	m_isAway = true;
	PutIRC("AWAY :Auto away");
      }
      return;
    } else {
      if (m_isAway) {
	m_isAway = false;
	PutIRC("AWAY");
      }
    }
    
    if (GetClient()->IsAway()) {
      GetClient()->PutClient(":irc.znc.in 306 + " + GetUser()->GetNick() + " :You are not visibly away yet.  " + tmpchr);
    }
  }

  virtual EModRet OnUserRaw(CString &sLine) {
    if (!sLine.Token(0).Equals("AWAY")) {
      return CONTINUE;
    }

    const CString sArg = sLine.Token(1, true).Trim_n(" ");
    if (sArg.empty() || sArg == ":") {
      GetClient()->SetAway(false);
    } else {
      GetClient()->SetAway(true);
    }

    DecideAwayness();

    return HALTCORE;
  }

};

template<> void TModInfo<CMultiAwayMod>(CModInfo& Info) {
  Info.SetWikiPage("multi_away");
  Info.AddType(CModInfo::NetworkModule);
}

USERMODULEDEFS(CMultiAwayMod, "Wait to set away until all connected clients are away");
