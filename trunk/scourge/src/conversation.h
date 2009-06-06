#ifndef CONVERSATION_H_
#define CONVERSATION_H_

#include "common/constants.h"
#include <vector>
#include <map>
#include <set>
#include <string>

class GameAdapter;
class ConfigLang;
class Creature;

#define INTRO_PHRASE "_INTRO_"
#define UNKNOWN_PHRASE "_UNKNOWN_"

class Conversation {
	
private:
	
	class NpcConversation {
	public:
		NpcConversation() {
		}

		~NpcConversation() {
		}

		std::vector<std::string> npc_intros;
		std::vector<std::string> npc_unknownPhrases;
		std::map<std::string, int> npc_conversations;
		std::map<std::string, std::string> npc_firstKeyPhrase;
		std::vector<std::string> npc_answers;
	};	
	
	std::string filename;
	GameAdapter *adapter;
	std::vector<std::string> intros;
	std::vector<std::string> unknownPhrases;
	std::map<std::string, int> conversations;
	std::map<std::string, std::string> firstKeyPhrase;
	std::vector<std::string> answers;
	std::map<std::string, Conversation::NpcConversation*> npcConversations;
	
	static std::map<std::string, Conversation*> cachedConversations;
	static std::map<Conversation*, std::set<Creature*>*> conversationRefs;

public:
	Conversation( std::string filename, GameAdapter *adapter );
	virtual ~Conversation();
	
	inline std::string getFilename() { return filename; }
	inline GameAdapter *getGameAdapter() { return adapter; }
	inline bool empty() { return intros.empty(); }
	
	static void unref( Creature *creature, Conversation *conversation );
	static Conversation *ref( Creature *creature, std::string filename, GameAdapter *adapter );
	static std::string decodeKeyValue( std::string key );
	
	char const* getIntro();
	char const* getAnswer( char const* keyphrase );
	char const* getFirstKeyPhrase( char const* keyphrase );
	char const* getIntro( char const* npc );
	bool setIntro( Creature *s, char const* keyphrase );
	char const* getAnswer( char const* npc, char const* keyphrase );
	char const* getFirstKeyPhrase( char const* npc, char const* keyphrase );		
	
private:
	void initConversations( ConfigLang *config );
	void Conversation::setGeneralConversationLine( std::string keyphrase, std::string answer );
	void setConversationLine( std::string npc, std::string keyphrase, std::string answer );
	void storeConversationLine( std::string keyphrase, 
	                            std::string answer, 
	                            std::vector<std::string> *intros, 
	                            std::vector<std::string> *unknownPhrases, 
	                            std::map<std::string, int> *conversations, 
	                            std::map<std::string, std::string> *firstKeyPhrase, 
	                            std::vector<std::string> *answers );	
};

#endif /*CONVERSATION_H_*/
