//
// Created by josh on 4/26/16.
//

#ifndef FAVOR_CONVERSATION_DATA_H
#define FAVOR_CONVERSATION_DATA_H

/*
 * Conversation starts and ends are fairly self explanatory; one-offs are messages sent outside conversations
 * (concretely, before the start or after the end) with no conversational response.
 *
 */
struct ConversationData{
public:
    double averageMsgCount = 0;
    double averageLengthTime = 0;
    double averangeTotalChars = 0;

    long totalConversations = 0;

    //From us
    long sentStartedCount = 0;
    long sentEndedCount = 0;
    long sentOneOffCount = 0;

    //From them
    long recStartedCount = 0;
    long recEndedCount = 0; //Conversations they dropped
    long recOneOffCount = 0;
};

#endif //FAVOR_CONVERSATION_DATA_H
