#include <vector>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include <queue>
#include "imdb.h"
#include "path.h"
using namespace std;

namespace {

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */
string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}

/**
 * *****************************************************************
 * Finding the shortest path using Breadth First Search 
 *
 */

const int MAX_DEPTH = 6;

// common types
typedef imdb               DB;
typedef path               NODE;
typedef list<NODE> QUEUE;
typedef vector<string> CAST;
typedef vector<film>   FILMS;

/**
 * *****************************************************************
 * class: visitRecord
 * A set of visited films and players
 * *****************************************************************
 */
class visitRecord {
    public:
    void recordVisit (const string& player)
    {
         players.insert(player);
    }

    void recordVisit (const film& film)
    {
         films.insert(film);
    }

    bool visitedBefore (const string& player)
    {
        if (players.find(player) != players.end()) {
            return true;
        } else {
            return false;
        }
    }

    bool visitedBefore (const film& film)
    {
        if (films.find(film) != films.end()) {
            return true;
        } else {
            return false;
        }
    }
    private:
    set<string> players;
    set<film>   films;
};

/**
 * *****************************************************************
 *  Method: addChildrenNodes
 *  ------------------
 *  Replace the current nodes in the queue with their children
 *
 *  @param db The database to use
 *  @param visited A record of visited players and films
 *  @param searchQueue The queue  
 *
 * *****************************************************************
 */
void addChildrenNodes(const DB& db, visitRecord& visited, QUEUE& searchQueue)
{
    ///////////////////////////////////////////////////////////////
    // Go through each node in queue
    int nodesInQueue = searchQueue.size();
    for (int n = 0; n < nodesInQueue; ++n) {
        ///////////////////////////////////////////////////////////////
        // Get player from node
        NODE* node = &searchQueue.front();
        
        string player = (*node).getLastPlayer();
        ///////////////////////////////////////////////////////////////
        // Get movies from player
        FILMS credits;
        if(db.getCredits(player, credits) && credits.size() > 0) {
            for(FILMS::const_iterator f = credits.begin(); f != credits.end(); ++f) {
                if (!visited.visitedBefore(*f)) {
                    ///////////////////////////////////////////////////////////////
                    // Get cast of players from movie
                    CAST cast;
                    db.getCast(*f, cast);
                    for (CAST::const_iterator p = cast.begin(); p != cast.end(); ++p) {
                        // Add new node if not visited before
                        if (!visited.visitedBefore(*p)) {
                            NODE newNode = (*node);
                            newNode.addConnection(*f,*p);
                            searchQueue.push_back(newNode);

                            // record visit
                            visited.recordVisit(*p);
                        }
                    }
                    visited.recordVisit(*f);
                    ///////////////////////////////////////////////////////////////
                }
            }
        } else {
            cerr << "addChildrenNodes: Films could not be found for a player" << endl;
        }
        searchQueue.pop_front();
        ///////////////////////////////////////////////////////////////
    }
    ///////////////////////////////////////////////////////////////
}

bool isNodeTarget (string target, NODE node)
{
    return (target == node.getLastPlayer());
}

/**
 * *****************************************************************
 *  Method: Breadth First Search
 *  ------------------
 *  search for target
 *
 *  @param db The database to use
 *  @param depth Current depth a record of visited players and films
 *  @param target The player to search for
 *  @param visited A class containing information on visited players and films
 *  @param searchQueue The current level of nodes
 *
 * *****************************************************************
 */
NODE BFS(const DB&     db,
         const int&    depth, 
         const string& target, 
         visitRecord&  visited,
         QUEUE&        searchQueue)
{
    // Check for target
    for (QUEUE::iterator n = searchQueue.begin(); n != searchQueue.end(); ++n) {
        if (isNodeTarget(target, *n)) {
            return (*n);
        }
    }

	if (depth < 1) {
        // return an empty path
        return path("");
    }

    // search deeper
    addChildrenNodes(db, visited, searchQueue);
    return BFS(db, depth - 1, target, visited, searchQueue);
}

/**
 * *****************************************************************
 *  Method: generateShortestPath
 *  ------------------
 *  find the shortest path from source to target
 *
 *  @param db The database to use
 *  @param source the starting player 
 *  @param target the target player 
 *
 * *****************************************************************
 */
path generateShortestPath(DB& db, const string& source, const string& target) 
{
    visitRecord visited; // a set of visited actors
	path p(source); // initial path

    QUEUE searchQueue;
    searchQueue.push_back(p);
    
    visited.recordVisit(source);
    
    path res = BFS(db, 6, target, visited, searchQueue);
    return res;
}

void getRandomPlayers (DB& db) {
    for (int i = 0; i < 10; ++i) {
        cout << db.getRandPlayer() << endl;
    }
}


}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

int main(int argc, char *argv[])
{
  if (argc != 2) {
    cerr << "Usage: six-degrees <data-files-path>" << endl;
    return 1;
  }

  imdb db(argv[1]);
  
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    exit(1);
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      //getRandomPlayers(db);
      path p = generateShortestPath(db, source, target);
      if (p.getLength() > 0) {
        cout << endl << p << endl;
      } else {
        cout << endl << "No path between those two people could be found." << endl << endl;
      }
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}


