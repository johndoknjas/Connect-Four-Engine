/* Version 59

- Functional change for V.59:
    - This version reverts back to the state of position.h just before the finishing touches were applied
      to V.57. The code can be found in commit f626a8e15ff2d10c209b62db16bd6494a1be8854 of the Versus Sim
      (with the three randomness lines commented out and the max_depth_limit changed to 9 of course). I'm
      also making it the V.59 code in the Versus Sim now.
    - So, V.59 is almost exactly as good as V.57, and only a little slower than V.58 (in range of 2 to 4
      percent). It should be noted that these measurements are from the Versus Sim, where V.57 and V.58
      performed fine. However, when I played against them, they weren't as superior to me as recent previous
      versions.
    - Since V.57 had issues against me, but this V.59 version did very well, a diff of V.59 and V.57's position.h
      code reveals that putting the (depth == impossible_depth) condition check higher in analyze_last_move
      (in an attempt to exit the function earlier) was the cause behind the engine playing worse against me.
      However, why this was an issue against me and not an issue in the Versus Sim is still unknown.
- Results:
    - In a depth 9 match against V.56, V.59 spent 0.0186562 seconds/move on average, while V.56 spent
      0.0228041 seconds. Speed increase of roughly 22.2%. All the trials were tied (433 games won each,
      134 draws).
    - Note that this Sim is from the aforementioned f626a8e15ff2d10c209b62db16bd6494a1be8854 commit
      from the Versus Sim. Also, in the Version 57 commit of the Connect Four engine
      (5cc4908f6503d752be5b6a27c8d96fc22aac8002), this match was mentioned in the notes - search for the phrase
      "code very similar to V.57".


----------------------------------------------------------------------------

-	Somehow, some squares amplifying 3-in-a-rows (and some 2-in-a-rows too I think) aren’t recorded. 
  This seems to be the case if such a threat is created in the root node. 
  So, maybe something to do with how the amplifying vectors are updated in main.cpp of the versus sim?
    o	It may be that this is more of an issue for the comp, since it seemed like less amplifying squares for 
      the comp were being printed out. Although this may just be a coincidence, not sure.
    o	See the critical-moves-idea branch (in the Versus Sim) for some test code I wrote for debugging this:
      -	Commit 0159ab1b57dfd72701dd8abb51b4e99efa7236c2 (my attempt to return out of analyze_last_move early 
        in some cases, which relied on the assumption that amplifying vectors would always store 3-in-a-rows).
      -	Commit 753cd6f349dc6726221b37a6d3e1541799742d61 (my attempts at debugging said improvement - involved
        printing out boards and amplifying vectors).
      -	When depth == impossible_depth, this should mean that the side to move is winning, since the 
        opponent failed to stop a critical move they had in the previous node. So, did_someone_win() 
        should always evaluate to false. But somehow it is sometimes evaluating to true 
        (at some point in every game), where ‘C’ is winning and ‘U’ has a 3-in-a-row 
        (and was on the verge of winning with a critical_move).

    - Also, I'm pretty sure this was an issue in previous versions, since if the early
      depth == impossible_depth check in analyze_last_move was causing this issue, the depth match
      between V.57 and V.56 would not have been exactly even in its score.

- Memory seems to be accumulating somehow. In the versus sim (depth = 9), V.51 and V.52 slowly
  use more and more memory; not enough to cause a crash before 500 trials are up (only like 1-2 GBs
  are used), but it's still odd. Also, when I play against the engine and spend a while making my
  move (allowing it to spend a lot of time thinking on my turn), memory usage is accumulating
  much quicker.
    - It seems like the TT is reset fully after each game, and other static data in the position
      class doesn't seem to be increasing in size in any way. Maybe try running valgrind? Not
      sure if that would help here.
    - Started investigating this stuff in the branch "debugging-accumulated-memory-usage".

- Profile the code, using gprof (https://www.youtube.com/watch?v=re79V7hNiBY). Then try to optimize any functions which are taking a large percentage of the time.

- Could try incorporating the V.58 idea. V.57 and V.58 did badly against you due to the early
  conditional check in analyze_last_move for V.57. So, it's reasonable to assume that the changes V.58
  added were not bad. Also, adding the V.58 idea on to the current version of the engine (V.59), should
  work just as well as it did when added on to V.57.
    - Adding the idea should yield a small speed increase in the Versus Sims.
    - Also make sure the engine continues to play very strongly against you.
      - It's possible that V.57 weakened the engine in games against you, and then V.58 somehow weakened it
        even further. But if you play it and there's no problems then all good.

- Value odd squares more than evens?
   - For the player favouring odds, obviously odds are good, but they also give the player the option to
     prefer evens. Say their square is on A3 - they move to A1, and now for the rest of the board, they
     essentially become like the second player, whose even squares are powerful.
   - Similarly, if the second player has an A3 square, then they could move to A1 and now have their
     odd squares elsewhere in the board be more powerful. 
   - And if the player wants to go back to preferring what they like by default (odds/evens),
     they can just fill up the file with their odd threat. In the aforementioned A3 example, on the player's
     turn they go to A2. Then, A3, A4, A5, A6 are left on the file, which is an even number and thus
     can't be used to waste a move (unless the player or opponent has an additional threat further up
     the A file).
   
   - Something interesting I noticed when playing against the engine is that if both players have two
     winning threats on A3 and E3, this alone will give the second player the win. E.g., first player goes
     to A1, second player goes to E1, leading to a zugzwang.
      - This ties in with the idea that one of the second player's odd threats allows them to switch
        to preferring their non-default type of square (so in this case even --> odd), enabling their
        other odd threat to win the game.
    
   - There could be something to adding up the rank values of a player's threats. So for A3 and E3, this adds
     up to an even value, and evens are what the second player prefers. Therefore, two odd threats are
     comparable to having two even threats for the second player.
      - For adding up threats' row values though, don't allow an additional threat to hurt a player.
        E.g., if the second player has A3, E3, G3 threats, this would add up to an odd number, but obviously
        the extra threat on G3 isn't detrimental.
    
    - Things aren't so clear if the opponent has a threat of their own near one of the player's threats.
      E.g., for A3 and E3, if the opponent has a threat on A2, then the aforementioned usefulness of A3 is
      essentially nil.
        - If the opponent has an A4 threat, then maybe A3 is still useful, in the sense discussed above?
          Since the player can go to A1, allowing them to prefer evens. Then, if the rest of the board fills
          up and the player hasn't won anywhere, they can just go to A2. Opponent goes to A3, player goes to
          A4, and the opponent's A4 threat has proved to be worthless.

- For columns (or sections of columns) that can no longer impact the game, it seems like a waste of resources
  to have the engine calculate different branches, some of which have pieces moving into some of these
  columns' squares, and others not.
    - One option is to avoid calculating at all in these columns, and just leave them empty (unless such
      columns are the only option left, or if the opponent has something like a vertical 3 in a row in this
      column).
    - Another option is to just fill the column with 'C' and 'U' pieces, and then to have the engine
      calculate as normal for the rest of the board.
    
    - The first option could be an issue when the engine is playing against a human, and it's losing, so
      the goal is for it to put up a stiff resistance. Doing this often involves preferring moves that
      don't do anything, but prolonge the game.
    - The second option can be an issue for the inverse reason; i.e., if the computer is winning against
      the user, it should try to finish the game quickly.
    
    - There are some other issues. E.g., for the second option, when would the pieces be placed into the
      column? If it's all done in the second constructor (before the minimax stuff begins), then what
      will the root position object's board look like? If it has the pieces in it after all the calculations
      are done, then this could be an issue if assisting boards in main.cpp need to get the object's board.

    - Also, if the first option is used, then if the opponent moves into one of these columns, the TT from
      the engine's previous think won't be able to be used. Although this doesn't matter much, since
      the current state of things will now be 2 ply deeper - most of the TT will be obsolete anyway.
    
    - If the second option is used, the engine shouldn't actually play into one of these columns for
      the actual move it wants to play in the game (at least not always). E.g., there may be some
      pressing matter to attend to elsewhere in the board.
    
    - For the second option, an even number of total moves should be made. I.e., the same number of 'C' and
      'U' pieces should be placed.
    
    - Will this stuff impact the multithreading done (where the engine thinks while it's the user's turn?).

- Consider storing a node's critical_moves vector in the TT, since this could save time by not having to
  call the find_critical_moves function for duplicate nodes.
      - Or, you could just store the number of critical moves, and also whether the player whose turn it is
        has any critical moves.
      - This is all the info you really need for the stuff you do in analyze_last_move. For using the actual
        critical_moves themselves, they'd already be stored at the front of the possible_moves vector
        for the TT duplicate.
      - You could then do similar behavior to what's seen at the end of analyze_last_move (with the minimax
        calls). Note that in the case of there being 1 critical move, if you've found a duplicate in the TT
        with a non-empty possible_moves_sorted vector, then it should already have the critical move at the
        front.
      - Earlier in analyze_last_move, the find_critical_moves function is called. Here, you could just
        loop through the TT earlier, and get the info you need (e.g., whether the player whose turn it is
        has any critical moves).

- For the possible_moves vector stored in a TT for a node, indicate somehow how many of the moves should
  be seriously considered (i.e., how many of the first i elements in possible_moves do not lose on the
  spot to the opponent by allowing them to play one of their critical moves).
    - If you do the above idea of storing critical_moves in the TT, then you may not need to store anything
      separate to indicate how many of the first i possible_moves should be seriously considered. Just get
      the size of critical_moves in the TT, and the duplicate node will then know to only go through the first
      i = critical_moves.size() elements in the possible_moves vector it gets from the TT node.

-	No need to find the next_square and other_next_square for the squares in 
  squares_amplifying_3_in_a_row for comp and user.

-	In the clean amplifying vectors function, remove any duplicate amplifying squares.

-	See if there’s some way to use the critical moves vector from a parent node to avoid 
  having to recalculate everything again for a child’s critical_moves vector.

-	For a square amplifying a 2-in-a-row, if it only has one next_square/other_next_square, 
  don’t count it if this overlaps with another 2-in-a-row square and next_square/other_next_square.
    o	If the first 2-in-a-row has both a next_square and other_next_square, still don’t count it 
      if all three (current square, next, and other_next) all overlap with other 2-in-a-row stuff.
    o	Basically, if this first 2-in-a-row’s squares were filled, and you could take the 2-in-a-row 
      away and the player’s position would still be as strong, then don’t count the 2-in-a-row.

/* Ways to build on Zobrist Hashing from Version 51:
  - Although the zobrist hashing increased the speed by 5%, there may be potential for more. E.g., the comment and answer
    to the following question suggest making the TT size a prime number:
    https://stackoverflow.com/questions/62541946/collisions-in-a-zobrist-transposition-table
    I'm still not exactly sure what the comment means by a power of 2 for the size "throwing away the upper bits of the hash value"
    though.
  - Could opt to no longer store the 2D vector/string board in the hash table. Instead, maybe just store a few additional
    zobrist hashes for each object in a bucket, using different randomly generated zobrist tables. This can resolve
    collisions. The probability of two objects being in a bucket, as well as two more of the same zobrist hashes, should be something
    like 1 million squared. Even higher, if for the secondary zobrist hashes, you take random numbers in the range of 1
    to the max size of a long long int (2^64).
        - OR: store two unsigned long longs as members of each node. The first long long is a 64-bit integer, where 42 of its
          bits = 1 when there is a 'C' in the corresponding square, and 0 otherwise. The second long long is the same size,
          and 42 of its bits store 1 if there is a 'U'. These two variables should hold the same data as a 2D vector of chars / string.
        - So, maybe it's more efficient to store and compare these two long longs in the TT, rather than the 2D vector or a string.
          Although, it's probably not a huge performance increase, since two long longs is something like 
          ~38% the size of a 2D vector of chars (64*2 bits vs 42*8 bits + vector overhead).
        - Note that for the two long longs, the way you'd keep track of the board state is by the parent passing a copy of the
          long longs to the child, and then depending on whether the comp or user has just moved, one of the long longs
          gets its corresponding bit (for the last_move square) set ("set" means make it 1, and in this case it would be 0 before
          since the square was previously empty).
            - Bit setting:
              - https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
              - https://www.geeksforgeeks.org/set-clear-and-toggle-a-given-bit-of-a-number-in-c/
              - Probably prefer the first answer from the SO link, since it seems to account for unsigned long long stuff.
        - You could also consider representing the actual board with two long long ints (so not just the one stored in the TT).
          Not sure if this would be more efficient or not though.
  - Could get rid of the indices_of_elements_in_TT vector, and just reset the vector after each game. If this is fast, then
    it would maybe be a bit efficient to do this (since now indices_of_elements doesn't need to have push_back called on it
    everytime a new bucket is used in the TT).
 
  - Note that making the TT bigger will reduce collisions (both for zobrist hashing and the old hashing method you were doing before), but using a TT
    at like size 10 million may actually slow down the engine. So for now, a size of around 1 million seems like a relatively good sweet spot.

  - Instead of using vector<bool>, use two unsigned long longs, and apply bitwise operations to them. Each bit represents whether
    a square would win for one of the players. Also, for unsigned long longs (64 bits), passing by value should (probably) be at least
    as efficient as passing by reference (which would likely be implementing using a pointer that is 64 bits). 
    Passing by reference has some indirection involved in accessing the value.

- When checking if a duplicate of a position exists in the TT (e.g., see the add_position_to_transposition_table
  and analyze_last_move functions), you could switch the order of the && check. So first check if the
  is_comp_turn bools match, and only then check if the strings match.
    - Idea is to try to get some performance gain out of short-circuit evaluation. If the positions differ,
      then it probably won't take long for comparing strings to figure this out. However, if the positions
      share a number of the same chars, then it would be quicker to check the bool (50% chance of it revealing
      they're not the same position immediately).

- In analyze_last_move, two for loops are used to go through the bucket. Although most buckets should be fairly
  small, this seems a bit inefficient.

- A few ways to build on V.54:
    - Could experiment with adjusting the coefficient for winning D3 threats for the player favouring odds.
    - If a normal square amplifying a 2-in-a-row (for the player favouring odds) is on D3, could also
      increase its value, even though putting a piece on that square wouldn't lead to an immediate win. This
      may not be beneficial, but you can experiment and see. If it turns out to not be good, you could try
      just increasing the value if it aids a 2-in-a-row that is horizontal. Since then, putting a piece on D3
      would lead to there being a square amplifying a 3-in-a-row on the 3rd row (i.e., an odd row).
    - Could experiment with increasing the value of D2 and D4 threats, if the player favours even squares.
      For D4, only do this if the opponent has no threat on D3 (similar to how in V.54, you only increased
      the value of a D3 threat if the opponent had no D2 threat).

- See 1.png in the position-images folder. There yellow is to move, and one of red's goals is to create a winning threat on F4 (an odd square, which red wants).
  If yellow plays to G4, red could go to G5, creating the threat. If yellow does nothing, then red could go to G4 and create the threat anyway 
  (although in this example yellow could then go to G5 and create a threat right below, but assume this isn't the case).

     - The square F4 (amplifies multiple 2-in-a-rows) counts as a number of amplifying squares, it has next_squares stacked on each other: G3, G4, G5.
     
     - So, a spot amplifying a 2-in-a-row should be worth more if it's next_square or other_next_square is directly on top of/below the next_square/other_next_square
       of another spot amplifying a 2-in-a-row.
	   - In find_individual_player_evaluation, it seems something similar is being done, where the engine checks whether the next_square/other_next_square of a 2-in-a-row 
             spot is vertically adjacent to a square that wins for the player.
		  - The feature being proposed here is like this, but seeing if the next_square/other_next_square is vertically adjacent to another spot's next_square/other_next_square.
                  - This would probably involve making 2D vectors (or bitstrings!) to store whether each square is the next_square/other_next_square of a 2-in-a-row amplifying spot.
                    One vector/bitstring each for the player and opponent.

- In find_individual_player_evaluation, this term is used a few times: "static_cast<double>(current_square.row + 1 + (*num_pieces_per_column)[current_square.col])".
  This gives a square a higher value based on two factors: how low it is on the board, and how many pieces are in its column below it. In essence, the more empty squares
  an amplifying square has under it, the lower its value. 
  However, if some of these empty squares can never help make a 4-in-a-row for the opponent, then they're not
  potential candidates for stopping the player from using their amplifying square. So, only count empty squares that can still potentially be used by the opponent.
  You could even only count empty squares that are currenly an amplifying square for the opponent. If filling an empty square would only result in the opponent getting a
  1-in-a-row or a 2-in-a-row, then it's not much of a danger to the player (at least for now).
  Or, maybe have some tier list. If a squares that can never win, squares amplifying a 1-in-a-row, 2-in-a-row,
  and 3-in-a-row. Having less enemy squares (and lower-tier ones) below a square makes its value be greater.

- If a move is forced (due to the oppponent threatening a 4-in-a-row), one idea is to not count it as increasing the depth by 1 ply. This will allow the engine to go
  deeper into this subtree. However, you'd have to figure out what depth to store in the TT, when to use a duplicate in the TT with x depth, etc.

- Get rid of 2D vectors, as accessing elements in them is relatively inefficient. Replace with 1D vectors/std::arrays, or strings. 
  E.g., the 2D vector<vector<char>> board is now a string (increased performance by over 25% in V.52).
  The random_values_for_squares_with_C/U can be a 1D array, and this should help performance a little since they get accessed by the third constructor. 
  Also the board_of_squares_winning_for_user/comp should be 1D (or even better, make these two vars be long longs, where each of the
  42 squares is represented by 1 bit, and gets bitset when the square is winning for the player).

- Sometimes when playing against the computer, it hangs for a long time (this doesn't happen that often though). I'm guessing this is mainly due to playing it
  on a faster computer, since occasionally it could finish the search for a fairly high depth before 1 second, and then iterate into a 1 higher depth search
  before the 1 second is up. If you want to test this, you could play against an older version of the engine on the current computer.

- Make the const variables in find_individual_player_evaluation const static variables of the class instead.
  This may give a small efficiency boost, since they won't have to be created each time that function is called.

- The board_of_squares_winning_for_comp/user could be updated in each node, using data from the last_move. The last_move could cause some squares to no longer be a winning threat, and also
  make new squares a winning threat. Then in find_individual_player_evaluation, the engine would already know which squares are winning. 
  One benefit of this is that it wouldn't need to wait and do a second for loop through the normal squares amplifying 2-in-a-rows, since it'll already know all the squares that win.
  However, all this may be complicated to do, and the return on investment could be low.

- Could try improving on changes made in V.55. The V.55 stuff gave a slight improvement, but I suspect there 
  could be room for further growth.
    - One idea is to test by keeping track of all positions evaluated by smart_evaluation. 
      There should never be any duplicates in this list, for the duration of an entire game. If there
      are duplicates, then this means your idea isn't fully working as intended (since no longer having
      a negative calculation_depth should mean a node which reached a position via a quiescence search
      no longer has a lower calculation_depth as compared to a node which got there "organically").
    - From a test I ran, it actually seems like V.55 only gets around 87% as many nodes as V.54 to
      evaluate to true in if statement inside the for loop of analyze_last_move. This is curious, since my
      change for V.55 had the intention of having the opposite effect. V.55 was supposed to catch more positions
      that had already been evaluated by a node that reached there via quiescence.
        - But interestingly, V.55 nevertheless seems to be very slightly faster and slightly stronger than
          V.54.
        - Anyway, run a test where you do this test again, but also record the number of nodes reaching
          analyze_last_move for both versions. If V.55 has like 87% as many nodes in the function, then
          obviously it's fine if it has 87% as many nodes evaluating to true in that for loop.
            - Although in that case, I'd imagine it would have a fairly significant speed increase over V.54?
    - Also, a few more notes under "Expectations" from the V.55 commit.

- Don’t create temp in add_position_to_transposition_table, until you’re sure you want to add it to the TT. 
  Should help efficiency a bit?

- Experiment with making C3 and E3 worth slightly more, similar to what you've done with D3 (but probably
  don't make the engine evaluate them quite as highly as D3). However, I haven't noticed the engine
  having an issue with under-evaluating C3 and E3 when I have played against it. So this experiment
  may not yield any improvements.

- If unordered_map is eventually used as the hash table (see below), then since the board is now a string, it could make a key for it. But zobrist hashing
  is probably more efficient (less work to do on each node) - unless you can find a way to combine zobrist hashing with unordered_map.
- May be tricky to implement the TT as a 1D array/vector, since the bucket sizes are variable. But consider
  using some built-in C++ hash structure.
      - Such as std::unordered_map. Best case complexity is O(1), worst case is O(n) (if your hash function was so bad
        that it stored all the data in one single bucket). So complexity should be quite close to O(1).
      - Another option is std::map, which you could experiment with using instead of unordered_map to see if it increases speed.
          - There's a chance map will be better if a lot of insertion/deletion is happening, but even here who knows.
          - unordered_map seems to generally be faster, when you have a big table and order doesn't matter.
- For any two elemnts (including strings), the odds of them having them same hash (from std::hash)
  approaches 1 / numeric_limits - so roughly 1 in 2 billion? This may not be correct, but if so then it means that almost all buckets
  should just have one element in them (meaning a for loop through the bucket will almost always
  only loop once - helps branch prediction). Although the loops you currently have may be changed,
  depending on how unordered_map works - don't know.
      - Problem(?): Turns out the engine goes through a few million nodes per game, so this means
        std::hash would be called millions of times on strings that are 42 characters in size.
        May have bad performance (but could still check it?).
      - Idea: if you use unordered_map, don't store a copy of the board (in a position_info_for_TT object).
        The chance of a hash collision is already very unlikely, so you could just store the hash value you're
        currently using, and use it as a second check when referencing an element in the unordered_map.
        This saves on having to copy a vector, the heap memory to store it, and the cost of comparing a board
        to a board in the TT to ensure it's the right element.
      - In the benchmarking-code branch, I'm now outputting statistics for the TT. You
        could continue to play around with it to see if there are any ideas to improve it.
      - It would be interesting to see which buckets in the TT get filled. E.g., are
        they mainly ones between 100,000 - 200,000, or 400,000 - 600,000, etc.
  
- Change the naming conventions for user defined types (classes, structs) to start with a capital letter
  for each word, and to no longer have underscores. See https://google.github.io/styleguide/cppguide.html#Naming, 
  under "Type Names".
  For any naming conventions you change here in the engine, change them in the Versus Sim as well. In some
  cases this may be difficult to do. E.g., structs in CommonClassData.h, or making the position class capitalized
  everywhere.

- Could store the starting positions played against the user in a textfile or DB (as two long longs maybe?),
  and also store the positions played in a trial of the Versus Sim. The point of this is to verify 
  that different positions are being played.
      - For the Versus Sim, the number of playable positions before ply 3 or 4 is fairly small though, so
        for the test here only consider positions at 5 ply and above.
      - Then, you could write a script that goes through the textfile / DB, outputting how many times 
        a position at each ply (5 to 7/8) has been featured in a trial over all the simulations. 
        The script could also verify that in each simulation, no starting position has been played twice.
      - E.g., say the script runs on the Versus Sim, and has recorded 10 simulations so far (where
        each simulation is something like 500 trials). If it says a position at ply 5 has been played
        10 times, this may be reasonable, as the number of playable positions at ply 5 is somewhere in
        the hundreds. So a position in a pool of hundreds being picked 10/(500*10) of the time is
        somewhat reasonable.
      - For testing the Versus Sim, make sure that a position used in a trial isn't counted for
        both games in the trial - should only be counted once.
        
      - Although note that since the random_shuffle stuff has been replaced with random_device, mt19937, 
        and shuffle(), the randomness involved in picking starting positions should be good now. So
        there likely won't be any benefit gained from doing all this verification (although it's possible
        of course).
          - Also, even if the computer were still only playing the same 80 positions against the user over all the runs,
            this isn't that bad in the first place.
          - In the Versus Sim, the same positions on each run definitely isn't ideal, although it's still a decent test of the
            engines' strength.
              - And in multiple depth_limit trials, the ratio of scores between the same engines has been different,
                so clearly they're not playing identical sets of positions on each run. 
              - Some positions with only a few pieces are being featured on many runs, but this is expected behaviour.
                  - And for these positions, the trial they're being played in (out of the 500) can vary from run to run.
  
- For generating randomness, you could create a class with static functions (one that returns a random
  number and one that randomly shuffles a vector). This class would have a static mt19937 variable that is
  initialized once using random_device. This mt19937 variable is then used in both of the functions.
  The benefit of doing this is that it makes the process for getting a random number/shuffle slightly simpler - 
  just call one of the two functions (without needing to create a random_device or mt19937 instance on the spot).
  Also, in cases where you're currently creating a random_device and mt19937 instance just to get one
  random number/shuffle, it kind of defeats the purpose of using the mt19937. If only one number/shuffle
  is needed, then the sequences of numbers in mt19937 isn't being used - just the one number that the
  random_device seed maps to.
  This is still fine though, as random_device is a good number generator. But making a static mt19937 that is
  initialized with a random_device seed, and then used for all randomness in the run of the program from then on,
  is the way to fully utilize the power of the mersenne twister.

      - Note that if you do create this class for randomness generation, you'll somehow have to make sure the 
        mt19937 static var is initialized before static stuff in other classes 
        (i.e., in the position class), if said static members need to be initialized with randomness. 
        I.e., the random_values_for_squares_with_C/U vectors are filled with random values in their 
        initialization, so they'd be dependent on the static function for generating random numbers (which
        in turn would be dependent on the mt19937 instance being initialized).
      
      - It's also okay not to do all this, and just continue generating random numbers/shuffles as you currently
        are (it should be all working fine as it is right now).

- See if, when in a time limited search, it's now beneficial in some new places to let the engine search, instead of
  relying on the DB. The reason for checking this out is that this new computer is a lot faster, so maybe there's some
  instances where the computer can calculate further than the DB goes, in certain positions.

- For declaring raw arrays / std::arrays, if you don't use new then they should point to the stack or to static memory. If such
  arrays are fields of the Position class though, and if you can't avoid declaring objects of Position on the heap (otherwise a 
  stack overflow??, not sure), then will these array fields point to memory on the stack, static, or heap?
     - In any case, a 1D list should have its elements stored contiguously, whether it be in any of these storage categories.
       So it should only have a marginal affect on performance, if its elements are on the heap (and the heap might be just as good or better in the case of
       contiguous items, who knows).
     - If the elements are on the stack or static, then this will only be an issue if there is some stack overflow (but then you'd know
       since the program will crash, especially after running the Versus Sim).

- For the "squares_amplifying_comp/user_2/3" vectors, the resize call each time the third constructor is run is a bit inefficient.
  Consider refactoring these vectors to be std::arrays, maybe at some big size like 100, and having the last element be an int saying
  how many of the array's elements are actually being used. So when adding/"deleting" from it, just change the value of this int.
  And then just make sure to only use elements that are within the bounds of what should be used in the array.

- Replace std::vector with std::array, where applicable.
- Try to avoid using the heap, ideally everywhere (maybe this is possible?). Whenever a unique pointer/shared pointer is declared, the heap is being used.
  For passing parameters to the 3rd constructor (recursively), passing a std::array (or a pointer to the std::array) should
  work, and allow modifications to the original if desired.
     - Note: Actually, only using the stack could cause a stack overflow. So maybe using the heap is needed.
     - But still try to replace std::vector with std::array, since the elements in std::array should be stored on the stack,
       even if the std::array pointer variable is on the heap (I think?).

- https://stackoverflow.com/questions/3628081/shared-ptr-horrible-speed
   - Consider passing shared_ptrs by reference, instead of by value (see the top answer).
   - Could also stop using shared_ptr altogether, and just use raw pointers (or even better, a reference!). E.g.,
     if all the nodes have a raw pointer to the same board on the heap, then even if there 
     is a memory leak, it would be insignificant.
   - Also, is using the heap even needed? If you have a pointer to a board on the stack, 
     such as a pointer to a string on the stack (string's chars will be on heap still though),
     and then pass that pointer to a child node, should work?
   - Or, instead of using pointers in the first place, pass a reference instead. This will allow you to avoid dereferencing all the time, which will make the code a bit
     cleaner, and may or may not increase performance (but definitely shouldn't hurt it?).
   
   - Maybe a better idea is to just pass stuff by reference to the 3rd constructor? E.g., for a string board, just
     make the parameter (string& boardP). This avoids overhead from shared_ptr, and also avoids having to dereference
     a pointer (not sure if this part actually will help though after the compiler's done its thing; but still, there is a clear benefit
     in that the code will be cleaner without having to deference things everywhere).

        - Each node will have a field string& board, and in the constructor just do board = boardP. Then when a node changes its
          board, this should just modify the same board all the nodes share.
              - Also, in Experiments/Indexing time with and without dereferencing - also timing reference assignment, I timed
                assigning a big string to a string&, and it looks like its O(1). So this should definitely be efficient.

        - Not using pointers shouldn't cause a memory leak (and even if it did, it would be highly insignificant). If the
          string is located on the stack, no problem. And if unique_ptr<position> causes the string field to be on the heap,
          then the destructor for the position class should call the string destructor.

        - https://www.geeksforgeeks.org/references-in-c/

        - Or, for stuff you pass by reference, could just make these members static. Just make sure that if two instances of position are being used,
          the board is updated / reverted to the state that the other needs when it's about to go again.
        - In think_on_game_position, when the find_best_move function is called, the static board would have to be updated.
        - Also, if the first constructor is called, the static board would be updated to be the empty board.
        - The board would only match one instance of position at a time, so if there's a case (like in main.cpp?) where the board needs to be retrieved from
          two instances of position, might be a problem. But I don't think this is an issue anywhere.

- See where inlining may increase performance (don't do excessive inlining though).
  Inlining may be helpful for small functions that are called many times.
  Use the inline keyword -- although note that this is just a suggestion to the compiler to inline,
  it doesn't force it to. To force gcc, also use: __attribute__((always_inline)).
  https://stackoverflow.com/questions/8381293/how-do-i-force-gcc-to-inline-a-function
    - Especially after compiling with -O2, gcc should be likely to inline your function if it will help.
      -O2 includes the -finline-functions flag (https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html).
      You can always check the resulting assembly with the link below:
    - https://gcc.godbolt.org/
       Good resource - allows you to see the assembly output by gcc, mapped to the C++ code.
    - Using the compiler flag -Winline with gcc will warn you when a function won't be inlined.

- An option to try multithreading is to make static variables into some pointer/reference types, and pass them from parent nodes to child nodes (like the board, for example).
  So, each thread would have its own TT. However, I don't recommend doing this -- will likely be very convoluted, and it's not clear whether this would even make the minimax
  algorithm more efficient in the first place. Even if this did improve the engine, maintainability would suffer.

----------------------------------------------------------------------------
Jan 2022 onward stuff above this line.



    ******IDEAS FOR FUTURE VERSIONS******:

    - When you're going to be done the project for a while, make the pygame exe. Don't do it for
      every version because it takes around 700MB on its own (even excluding the DB).
        - Instructions for creating the exe are in the notes of interface.py.

    - Somewhat small bug. At the start of a game, the starting position
      suddenly changed to a new one. But hasn't seemed to happen again, may just
      be a one time thing from when I was testing it.

    - A small bug... sometimes this exception now gets thrown:
        - "row variable in play_game() is smaller than 0, or there is no move chosen by the user recorded."
            - This means the user's move isn't valid in C++'s "assisting_board", even though
              the Python program seemed to verify it was legal before sending it over.
              So maybe, add back in the C++ program's verification of the move's legality too.
              If it says it's not legal, get the user to click again to enter a move back
              in the Python program.
           - I've wrote some code to output a detailed error message to a txt file if it happens again.

    - In the think_ function, for the early return block where thinking_time < 0.000001,
      consider resetting the TT (IF !surpassedDB - if surpassedDB, then DON'T reset the TT).
      Since without doing this, a few "bad" elements in the TT remain, since these were put
      there in the era when the DB was still used over the engine. Of course, the number of
      these elements added to the TT is miniscule, since the search right before this if statement
      was depth_limit = 1.
         - In addition, let this if statement run for both depth_limit and time_limit modes.
           So just eliminate the "max_depth_limit != UNDEFINED" sub-condition.
         - The reason to let it run for the time_limit mode as well is that without doing so,
           control goes to the while loop, doesn't run an iteration, then goes past the end
           of the loop, resets the TT (if the DB has never been surpassed), and then does
           some SELECTS. These SELECTS are unnecessary on a thinking_time = 0 think, and
           take up a finite amount of time (although it should be very miniscule).
        - Besides, resetting the TT and returning right in that if statement makes control easier.
          Now depth_limit and time_limit modes terminate at the same spot, with the same behaviour,
          if thinking_time = 0. Don't have to trace the control from time_limit throughout the function.
        - Note that in the if statement, for the embedded if statement testing if
          !surpassedDB, before resetting the TT, this essentially also checks that the DB
          wasn't surpassed on the depth_limit = 1 think just done before the if statement.
          Since if the engine was able to surpass the DB with a depth_limit = 1 think, it would
          have surpassed it before on a depth_limit = 8, 9, 10, etc think. Therefore,
          !surpassedDB --> the engine didn't just surapss the DB with the depth_limit = 1 think.

        - When testing the new version with these changes against V.44, look for the following:
            - In the depth_limit match, V.45 should be equal, or possibly better (doubt it though).
              Since the change here is that the TT is reset after a thinking_time = 0 think,
              so that should only help or keep things equal (shouldn't make things worse...).
            - In the time_limit match, only aiming for equality. The behaviour is essentially
              the same, just without the additional stuff like SELECTS (that aren't used when
              the think is for thinking_time = 0). Also, V.45 may have a very slightly smaller
              average time for thinks where the thinking_time = 0.


    - In future versions, the engine may be stronger in its evaluation function than
      V.41 was (i.e., the engine used to generate Database C). If this is the case,
      then if a think_ reaches 18 ply, it would be slightly preferable to use the
      engine's think rather than the DB.
         - So in the think_ function, you would change the <= 18 at the end of the loop to < 18,
           the <= 16 in the loop (for testing whether to break) to < 16, and in the condition
           for the while loop (for the sub-condition on depth_limit mode), change > 18 to >= 18.
         - This makes it so that the goal for "surpassing" the DB (i.e., prefering the engine)
           is at least 8 ply, not at least 19 ply.
         - Note that if a future version becomes faster/more efficient at thinking,
           but still evaluates positions the same, then should be completely equally as good
           to still use the DB at 18 ply. No reason to prefer the engine at 18 ply here.

    - You could try always resetting the TT at the end of the think_ function ALWAYS
      (regardless of whether the DB was surpassed or not), just in case this somehow
      improves the engine. Test it in a depth_limit match against whatever the current
      version is at the time of reading this.

    - Try to find out why modifying the TT, on runs in the think_ function where the
      DB ends up being used, appears to be detrimental. It seems like this modified TT
      is actually worse when used on a subsequent run in think_, where the result is
      important as the DB isn't used.
         - If you can solve this mystery, fix the TT and make a new version (V.45), and
           ensure it is at least around equal with this version (V.44) in a depth_limit match.
                (since V.44 just did a hacky fix to the underlying problem, for depth_limit mode)
         - Also ensure it is as superior to previous versions in a depth_limit match as V.44 is.
           Then, test V.45 in a time_limit match against its predecessors. Should be clearly
           superior.

    -  In addition to passing a board to child nodes, pass a string to the constructors as well. Pass it like
       the board is passed - i.e., as a shared_ptr for efficiency. Then remove char in string representing
       last_move at the end of the constructor.
       The point of also passing this string is to instead add the string to a "position_info_for_TT" object
       when inserting into the TT, instead of adding the vector<vector<char>> board. The idea
       is to make it faster in the add_position_to_transposition_table() function.
       Then when using the TT, just check if the strings match (and whether is_comp_turn matches too of course).

    -  In analyze_last_move, in the first for loop through the bucket in the TT, keep track
       of the index of the duplicate (if it exists), and a bool saying whether a duplicate
       was even found. Then in the second for loop near the end of analyze_last_move, you
       know that index in the bucket (if the bool = true) is the duplicate. Just check
       if its !possible_moves_sorted.empty(), and then set possible_moves of the calling object
       equal to it.
            - For this, just see the difference in analyze_last_move between the Version 43
              and Version 41 files, in the folder "Testing there are never more than 2 copies..."
              folder. Note that the folder name no longer has anything to do with testing
              for >= 2 copies.
            - Could just copy the difference there verbatim, for analyze_last_move.
            - In a few trials I ran there, there was about a 1.5% speed gain.



    -       A   B   C   D   E   F   G

        6 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|
        5 | C | C | C |   |   |   |   |
          |---|---|---|---|---|---|---|
        4 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|
        3 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|
        2 |   |   |   | U |   |   |   |
          |---|---|---|---|---|---|---|
        1 |   |   |   | U |   |   |   |
          |---|---|---|---|---|---|---|

            - Assume the D5 square was considered winning for the comp with the D algorithm.
            - However, if it's the user's move here, they could go to D3, forcing D4, and then they could go to D5.
            - So in such a situation where the user has a vertical two in a row three below the D square and it's their move,
              don't evaluate as winning for the comp (or winning for the user is the situation is reversed).
                - Unless the D4 square is also winning for the comp, then obviously evaluate as winning.
                - Think of any other exceptions where this situation would still be winning.

    - For this kind of situation:

            A   B   C   D   E   F   G

        6 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|
        5 |   | C | C |   |   | C | C |
          |---|---|---|---|---|---|---|
        4 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|
        3 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|
        2 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|
        1 |   |   |   |   |   |   |   |
          |---|---|---|---|---|---|---|

          Only one square amplifying a 2-in-a-row should be counted here, since either pair of C's could
          be taken away with no detriment.


    - Consider doing the D algorithm anytime you've reached a quiescent state in the search tree? Doesn't have to be only at
      depth >= depth_limit.
        - Point of this is to save time by not going down branches which you can already evaluate as winning/losing immediately.
        - Shouldn't improve the engine in a depth_limit match, but could in a time_limit one?

    - Try to export the D algorithm to other columns as well? Might be tricky.

    - When user is thinking, clear out obsolete positions from the TT? I.e., positions with less pieces than the current position.
      Maybe don't do this every time the user thinks, but around every 5-10 turns?
	  - If the comp is in the middle of cleaning the TT and the user moves, it's fine. At least some of the TT's memory has been released.
	  - Doing this periodic cleaning could make it safer for the TT to muse longer than 10 seconds, since less chance of a memory overflow.

    - If the user decides the thinking_time to be, say, 1.0 seconds, get the computer to think no longer than 1.5-2.0 seconds.
  	  - Do this by getting it to return from minimax if a static flag in position is raised saying the time limit has passed 1.5.
	  - Use another thread in the think_ function to keep track of when the time passes 1.5, and then sets a static bool flag to true in position.
          - So if the comp is currently in depth 11 calculations, it stops and only uses its depth 10 calculations.
	  - In each loop iteration in think_, do something like unique_ptr<position> temp = think...**call constructor**
	  - Then, on the next line check if the thread has raised the flag saying the time limit has been exceeded.
		- If it hasn't, do pt = move(temp);
		- If it has, don't do pt = move(temp) since that search iteration was stopped. pt is then returned with only its depth 10 calculations.

    - Only value an odd/even square amplifying a 2-in-a-row if one of its next square(s) (either/both?) are also the correct odd/even.
      I'm not sure whether this would be beneficial or bad.

    - If a square amplifying a 2-in-a-row can be played immediately, as well as its next square(s), then it shouldn't be worth anything?
      (unless there are squares amplifying a 3-in-a-row for the player or opponent directly above).
	   - The logic behind not valuing these squares is that when the player plays in either the current_square or next_square, the opponent immediately
             fills the other, preventing the player from ever winning with a 4-in-a-row from that group.

    - Play around with the coefficient used to multiply a square's value if it's odd/even. So far 1.75 works very nicely,
      but it's possible an even value is better.

    - If a row barrier is in a column (C or E) then don't look at moving in columns in
      the smaller adjacent group unless...
          - Opponent is threatening a 4-in-a-row (or comp is).
          - Moving in some empty squares in the columns could give comp or opponent a 4-in-a-row involving some square underneath the row
            barrier in column C/E. Note that this square underneath the row barrier could have a piece in it. If it's empty,
            assume a piece is in it.

      If a node is stored in the TT, its possible_moves vector should be stored with it. Then a node that accesses the TT will know
      not to bother with some columns in its OWN search.

      If the opponent actually plays in one of these columns in the game, then the resulting position won't be in the TT and the comp will have
      all columns in its possible moves vector by starting "from scratch" in constructor 2.

      Have array of bools for where each column has been ignored? Array passed on to child nodes, stored in TT.


        A   B   C   D   E   F   G

    6 |   |   |   |   |   |   |   |
      |---|---|---|---|---|---|---|
    5 |   |   | X |   |   |   |   |
      |---|---|---|---|---|---|---|
    4 |   |   |   | X |   |   |   |
      |---|---|---|---|---|---|---|
    3 |   |   |   |   |   |   |   |
      |---|---|---|---|---|---|---|
    2 |   | X | X | X |   |   |   |
      |---|---|---|---|---|---|---|
    1 |   |   |   |   | O |   |   |
      |---|---|---|---|---|---|---|

      Say a row barrier was at E4:
	- For E3, E2, E1, assume piece X is already there IF square if empty. If enemy piece there, ignore it.
        - For E3, F2 could help but column G is useless.
        - For E2, both column F and G are useless.
        - For E1, cannot do anything. Note that if E1 were empty, F and G may be useful for it.
        - So ignore column G, since it's useless on ALL the squares below row barrier E4. Column F is useful on at least one square (E3) so can't ignore it.

        - Basically, column F is NOT useful for a square if any of the following:
		- square holds enemy piece.
                - The square on the other side (so for E2 it would be D2, horizontally speaking) is an enemy piece.
                - There are three of his own pieces in a row on the other side (D2, C2, B2, for E2).

        - Column G is NOT useful for a square if any of the following:
                - square holds enemy piece.
                - square in between on F holds enemy piece.
                - The square on the other side of the E square is an enemy piece.
                - There are two of his own pieces in a row on the other side (D2, C2).

        - So either column F/G not useful if E square holds enemy piece or if square on other side of E holds enemy piece.
        - In addition, F specifically not useful if three pieces in a row on other side.
        - In addition, G specifically not useful is two pieces in a row on other side, or if the F square holds enemy piece.
        - Run this algorithm for all E squares under row barrier to see whether F/G is not useful for all of them. If so, ignore F/G.

        - Note that all this can be done recursively. See whether columns potentially useful for E3 and call recurisvely on E2.
          Note that a column only has to be potentially useful for one square (under RB) to NOT be ignored.

      If possible_moves vector's size = like 2 or 1 but the game is not over, manually rebuild the possible_moves vector to reincorporate any abaonded columns.
      Then set a boolean flag to never try to do this algorithm again in child nodes (this flag should also be stored in TT?). The flag also tells you that there's
      no need to check if the possible_move vector's size = 2 or 1 in child nodes anymore, since all possible_moves have been reincorporated.

      Only bother doing all this if the row barrier in column C/E is on second/third/fourth lowest square? Idk

      Point of all this is to obviously lower the branching factor.



    -------------------------------- After July above




     - Improve the hash function. Right now you have it where top-right squares are given the highest base value, before being
       multiplied by a constant for 'C', 'U', or ' '. But many of those squares are all left empty until the game progresses enough.
	  - Update your algorithm to "value" squares that could be played in within a few moves. This depends on the row value of
            the highest pieces currently in each column.
          - Doing this should allow more different boards to get different hash values, lowering the avg. size of each bucket in the TT.
          - As an aside, run a test seeing what the avg. size of each bucket is in the TT.

     - If a square amplifying a 2-in-a-row has no pieces underneath it, then give it many or few points, depending on if it's the player's or
       opponent's turn.
          - If it's the opponent's turn but the player has an 'A' right above this square, then give the player lots of points.

     - For the if and else if group (0.25 and 0.75), only allow their conditions to be tested if (meaning, put them both inside the following
        big if block):
        - The square is NOT involved in a stacked threat situation (either as the top or bottom square).

      - Take out the else if for *0.75 in smart_evaluation(). Fine if both happen, since it's indicative of a square being
        one of those tit-for-tat columns.

      - Experiment with adjusting values for a 3-in-a-row with next_square under a fellow 4-in-a-row square. Also experiment
        with not completely discarding squares amplifying a 3-in-a-row directly below the opponent's 4-in-a-row square.

      - Make a third version of the "Generating File of Playable Positions" project. The current version of it right now uses
        a Version you made in October to select positions.
            - In the new version, replace the engine with this V.23 Engine. Also allow for positions with up to 8 pieces to be generated,
              and not just an even number of pieces! (although maybe you already allow for odd numbers, I don't know).
            - Then, after running overnight, copy the file of positions generated and replace the playable positions text file in this project.

      - For something like:    | | |X|X| | |

        - Count ALL FOUR of the empty squares as squares amplifying a 2-in-a-row....
            - The inner 2 squares you obviously already count, and they each have two next_squares.
            - The 2 outer squares you do not count yet. They only have ONE next_square (the respective inner square), and "other_next_square"
              is UNDEFINED.

      - For something like:  | |X| |X| |

        - Count ALL THREE of the empty squares as squares amplifying a 2-in-a-row.
            - You already count the inner square, which has two next_squares.
            - The two other squares you do not count yet. They each only have one next_square (the inner square).

      -  X
          |  |
            |  |
               X

            In the diagram above, count each of the squares as their own "amplifying 2-in-a-row" squares, since if it and its
            neighbor get filled, a 4-in-a-row is created.
                - If its neighbour (i.e., its "next_square") gets filled with an enemy piece do not count square.
                - If its neighbour gets filled with another X, obviously count the square as amplifying a 3-in-a-row, as you already do.
                - Not sure what "other_next_square" would be for each of these amplifying squares. Probably UNDEFINED.

      - squares amplifying a 2-in-a-row that can be filled in ONE MOVE should be treated differently (either positively or negatively).
        Change quiescent search requirements to also allow no squares amplifying a 2-in-a-row to be able to be filled immediately?

      - squares amplfying a 2-in-a-row that cannot ever make a 4-in-a-row (since outer 2 squares are filled with enemy piece
        or OUT-OF-BOUNDS), should NOT be counted.

      - Go through main.cpp and make it better? Esp. in play_game() -- make it think for 0 seconds where appropriate.

      - Play with adjusting the 0.5 and 0.75 parameters in smart_evaluation().
            - For the 0.75, I've confirmed it's most likely beneficial to have (as opposed to no parameter for it), but play with increasing
              it to something like 0.9.
                - With the parameter at 0.75, Version 17 did well against Version 16. Without the parameter (so effectively 1.0),
                  Version 17 still did well against Version 16 but not quite as well.
                - I'm assuming there may be a parabola here, with the absolute max between 0.75 and 1.0.
            - For the 0.5, not sure if it would be better raised or lowered.
            - If possible, try to fit data points to some function.

      - For evaluating a position, give a small amount to the player whose turn it is to move in that position. Since the Engines now think
        according to a time limit and not a depth limit, some positions will have comp to move and some will have user to move.
            - Actually, even when depth limit was used for thinking, quiescence search sometimes/often caused the search to go further.
            - So even then, giving the side to move a small amount (like +/- 5 or something) would have been useful.

      -	For a stacked threat of two 3-in-a-rows above the opponent's own amplifying square for a 3-in-a-row,
            ensure the two groups have equal value, if you can!

            -	The group of two above should NOT be worth more.

      //////////////////////////////////////////////////////////////////

            EDIT: For below idea, it actually isn't that good. Sure, the square amplifying the 2-in-a-row, can't be filled, but one
            of its next square(s) should be, resulting in a square amplifying a 3-in-a-row RIGHT BELOW the oppponent's 3-in-a-row square.
            Such a scenario gives the player a square amplifying a 3-in-a-row, and mostly nullifies the opponent's square as well.
      - If a player has a square amplifying a 2-in-a-row, do not count it if the opponent has a square the makes a 4-in-a-row directly above it.
            - Reason: if the player actually uses this square, the opponent wins immediately.
            - Of course, if the player's square amplifies a 2-in-a-row and connects with at least a single piece, then obviously count it
              since it creates a 4-in-a-row.
            - Note that it's fine if a next_square is directly below one of the opponent's squares making a 4-in-a-row. Since here,
              next_square isn't the one adding to evaluation. It's only there to see if the player's square amplifying a 2-in-a-row
              could POTENTIALLY win by filling in next_square (or already would make a 4-in-a-row if next_square is filled).
                - In this potential scenario, filling in next_square wins immediately, so the opponent's amplifying square above is moot.
            - You'll do this probably near the end of smart_evaluation():
                - Go through each player's amplifying_2_vectors. If the square does not have an 'A' in the player's copy_board
                  (or any other char representing a square amplifying a 3-in-a-row?), then it is a normal square amplifying a 2-in-a-row.
                - If so, then check the square directly above in the OPPONENT'S copy_board. If this square is in-bounds and has
                  an 'A', then that square would give the opponent a 4-in-a-row.
                - So don't give points to the user.

      - If the player has an amplifying 2-in-a-row where one of the NEXT_SQUARES is the square right below/above one of the player's
        square amplifying a 3-in-a-row, give the player points. This is because if the 2-in-a-row is amplified into a 3-in-a-row, the player will
        have a stacked threat set up.
            - You will probably implement this near the end of smart_evaluation, as you're running through the player's
              info_for_amplifying_squares.
            - IMPORTANT: Create a new struct called "treasure_spot_and_value". Should be used instead of coordinate_and_value in smart_evaluation(),
              in order to store the next_squares of a spot.
            - For each spot amplifying the player's 2-in-a-row, check if the player has an 'A' in its copy_board below/above one of
              the 2-in-a-row's next_squares.
                - If both next_squares have an 'A' below/above them,
                  give points for both 'A' (should not be penalized for having 2 separate instances).
                - Multiply the square amplifying the 2-in-a-row by some constant (maybe 1.5 its current value?).
                    - If this square has two next_squares that are both above/below an 'A', multiply the square amplifying 2-in-a-row's value
                      by the constant twice. So (square's current value)*(1.5)*(1.5).


      /////////////////////////////////////////////// ONE BIG IDEA:

      - In initialize_row_barriers, create two copy_boards for comp and user.
        - Send these copy_boards to find_winning_squares() by reference (one appropriate copy_board sent in each function call).
        - In find_winning_squares, update the copy_board with any winning squares found, by making the square in copy_board = 'A'.
        - Do NOT remove duplicates in find_winning_squares, not necessary.
        - Back in initialize_row_barriers, run through each squares_winning_for_player vector, doing the following:
            - If the same square in the opponent's copy_board also stores 'A', make the square a row barrier (if there's not a lower barrier yet).
            - If the square above in the player's copy_board also stores 'A', make the square a row barrier (if there's not a lower barrier yet).
                - Note that this is the idea behind the bottom paragraph.

        - Four additional ideas I came up with:
            - Make copy_board_for_comp and copy_board_for_user early on in smart_evaluation(). Then pass it by reference to initialize_row_barriers()
              and find_winning_squares(), if needed. Add any 'A' to it, but then before passing back to smart_evaluation(), make sure
              all 'A' are removed! copy_board should be as it was before it left smart_evaluation().
                - This is all just to save time on having to not make two more copies of 2-D vectors.

            - In initialize_row_barriers(),
              when checking if a square is a row barrier, see if it's an amplifying square for a 3-in-a-row for both sides before seeing
              if it's the bottom square of a stacked threat for one player.

            - In initialize_row_barriers(), run through the first player's squares_winning vector. Check if it qualifies for one of the two
              requirements to be a row barrier. If it's the bottom square of a stacked threat, add the top square to the special vector.
              Then run through the second payer's squares_winning vector. Only check if these squares are the bottom squares of a stacked threat,
              since if it was a square that both players point to, I would have already caught it when running through the first player's vector.

            - At the end of initialize_row_barriers(), run through the special vector of amplifying squares that I decided to keep since they
              were the top square of a stacked threat. Now, check if they still hold that status. If either of these are true:
                - The row barrier of that column is NOT directly under the amplifying square anymore, or
                - The row barrier of that column is an amplifying square for both players

                Then do not keep this amplifying square in the special vector. It should not be counted due to being inaccessible in the game.

      - The bottom square of the two squares in a stacked threat coefficient IS A ROW BARRIER!
         - It is essentially impossible for play to continue above the bottom square, unless the player decides not to win on the spot.
         - Use this fact to update the initialize_row_barriers() function.
         - An amplifying square for both sides is obviously still a row barrier, but the bottom square in a stacked threat for one player also
           qualifies.
         - Note that this also solves the problem of not counting things like stacked threats with 3+ squares, since the squares
           above first two (specifically, above the bottom of the first two) are not counted due to row_barriers.

         - However, note that so far, this algorithm prevents the top square in the stacked threat from being counted. So,
           in the initialize_row_barrier function, record this top square in some vector. Then:
            - Give it a normal value for 3-in-a-rows in find_individual_player_evaluation (don't give stacked_threat bonus points value).
            - Make sure you run through these special squares FIRST in find_individual_player_evaluation, in order to make sure
              the copy_board holds an 'A'. Allows the bottom square in the stacked_threat to qualify for the bonus points.

            - Note now that in find_individual_player_evaluation(), you only have to check if a square amplifying a 3-in-a-row has an
              'A' above it, not below it. This also removes inconsistencies / "randomness" with which square in a stacked_threat gets
              assigned the bonus points.
                - However, I'm still checking above and below just to be safe, even though there should never be an 'A' below.

      /////////////////////////////////////////////// ONE BIG IDEA

      - If a player has a square amplifying a 2-in-a-row, where one of its next_squares is directly below one of the opponent's 'A', then
        multiply the player's square's value by some constant, like 1.2. If the player's square has both of its next_squares directly under
        two of the opponent's 'A', multiply by the constant twice (*1.2*1.2).
            - Reason for this whole idea: If the player turns the 2-in-a-row into 3-in-a-row, then the opponent's square amplifying a
              3-in-a-row will have its value severly lowered (it will be *0.25, currently!).

      - If comp is winning find shortest path to win. If for some reason this is way too hard to do, at least make the computer
        choose a move if it wins on the spot!

      - If comp is losing, play the move that takes the longest to lose. Even though the comp doesn't go for any lose-in-1 if it can
        help it, it should be putting up the tougest resistance (not just the "not easiest" resistance).

      - Used anything you learn in CMPT 225 (open-indexing hash table, quadratic hashing, etc).

      - Add a bool attribute storing whose turn it is for the "position_info_for_TT" struct. Then when adding a position to the TT
        (in the add_position_to_transposition_table() function) and when checking if a position is in the TT, make sure to include
        this bool variable.
            - The reason is to avoid treating a position the same if its the comp's turn in one and the user's turn in the other.
              This should never happen since in the 2nd constructor (which includes whenever a new game starts), the TT is reset,
              apparently including positions even with INT_MAX or INT_MIN evaluations, although double check this.

            - So while there should never be a problem, it wouldn't hurt to include the bool variable, just in case you've missed something.


      - Apparently the order of squares in each of the amplifying vectors affects the evaluation calculated
        in smart_evaluation(). See where this could be the case, and fix it if possible.
            - It should be noted though that with transposition tables, the same position (arriving via a different
              move, so different order of amplifying squares) will never have to be evaluated twice.


      - Only randomize the possible_moves vector in constructors 1 and 2 (since the position object created in them
        directly decides what move the comp plays in the actual game).
            - In constructor 3, optimize the order of moves in possible_moves. The thousands of positions created
              in constructor 3 are just for the minimax process (i.e., finding evaluations). It doesn't matter how these
              evaluations/numbers are found, so efficiency is key here. The order of possible_moves for position objects
              created in Constructor 3 don't affect the move ultimately chosen by the computer, just the speed by which
              it happens.

            - A FEW OPTIMIZATION TECHNIQUES (Technique 2 is an idea for Version 10, and may allow depth_limit to
                                             be increased):
                1) Each time in analyze_last_move(), re-arrange the possible_moves vector to store moves that can
                   make a 4-in-a-row immediately.
                   DONE

                2) In the 3rd constructor (NOT THE 2ND AND 1ST!!) re-arrange the possible_moves vector to store moves
                   that, on average, tend to end up being the chosen move in the minimax calculations. To figure this out:

			  STRATEGY #1:
			  -------------
			  - Favour moves that are closer to the middle column (D) and have more pieces in a 3x3 square around them (where the move
                            is in the centre of the square).
				- Not sure whether to include all pieces ('C' and 'U') when counting in the 3x3 square, or just similar pieces.

			  - Maybe favour moves that are lower on the board (closer to the bottom row)? Not so sure about this one.....

			  - To discover more traits of good moves, see Strategy #2.



			  STRATEGY #2 (Investigating via Simulation):
			  --------------------------------------------
                        - Run a Versus Simulation, and for one (or both) of the Engines,
                          each time a move is "chosen" in its minimax process, record its
                          column & row in some static variables. You could even record the data to a file that keeps growing
                          in size over multiple simulations, giving you more and more accurate data.
                        - Note that each possible_move should be given equal fair chances to be examined sooner in the
                          minimax process. So, make sure to randomize the possible_moves vector in the 3rd constructor!
                          This won't allow randomness to affect what move an Engine actually chooses in the game, but it
                          will allow each move in possible_moves to have a fair chance of being looked at first.
                            - Then, if, for example, a particular column is "chosen" more in the minimax process, this
                              tells you it's because that column tends to be better on average, rather than because of
                              the order of moves in possible_moves.
                        - Then at the end, print all this data out (or print some averages from the data in the file).
                          This should give you an idea of which columns & rows tend
                          to be more successful (and thus should be tried first by being at the front of the possible_moves
                          vector).
                            - Note that the row value is relative compared to other moves available. For example, if a chosen
                              move has row value of 4, record how much higher/lower it is than the other moves that
                              could have been chosen. Do not just record "4".

                NOTE: Optimization Technique 1) and 2) are constantly overwriting each other...
                    - Technique 2) in Constructor 3 re-arranges possible_moves with moves that work on average. Then it
                      calls analyze_last_move() and...
                    - Technique 1) in analyze_last_move() puts moves that concretely work in the current position at the front.
                    - THIS IS GOOD. The result is a possible_moves vector with any moves that concretely work in the position
                      at the front of the vector, followed by moves that have a good chance of working on average, followed
                      by the scrappy moves at the back. Alpha-beta and minimax pruning should greatly work here!


        **NOTE**: The suggestion below is kind of useless if you do iterative deepening, since the depth searched is just how much
        time is available in an arbitrary time constraint you give to the Engine.

      - For the position class, make a static constant variable called X that equals breadth^depth, or 7^depth_limit. This is theoretically how many
        moves the Engine looks at when it thinks, if it weren't for any minimax/alpha beta pruning.
		- Then, each time the opponent moves and the Engine thinks of the new position (i.e., Constructor 2), check the size of possible_moves vector.
                - If possible_moves vector's size is less, keep incrementing depth_limit as long as possible_moves.size()^depth_limit is close to X.
		- This essentially lets the Engine increase its search depth, if one or more column(s) are full (decreasing breadth).
                - But, the Engine can never much slower than it was before, which is where the check for possible_moves.size()^depth_limit being close to X comes in.
                - By "close to X" I mean less than 2X ish. As long as the runtime isn't double as long, it's fine, since the engine is usually pretty quick.
                - Note that this means depth_limit should still be static, but cannot be const anymore since it sometimes gets updated in Constructor 2.

		- IMPORTANT UPDATE: Alpha-beta pruning is more effective when the depth limit is higher. So, let's say there were 7 possible moves, and depth limit
                  was 9. Theoretical max value of #moves = 7^9. Then, let's say there's only 3 possible moves, so we make depth limit = 20...
                  New theoretical max value of #moves = 3^20, which is 86 times greater than 7^9.

		  HOWEVER: if we assume Alpha-Beta pruning cuts down search depth by half (on average), then we get 7^4.5 and 3^10 for average number of moves
                  looked at. 3^10 is only 9.3 times greater than 7^4.5. So still not ideal whatsoever, but not nearly as drastic as 86 times greater.

                  So, incorporate this in. Maybe make a new search depth limit acceptable if, after alpha-beta prunes around half the search depth, the new average
                  #moves is <= 2x #moves at the start of the game with alpha-beta pruning. This will require trial and error, since I don't know
                  how effective alpha-beta pruning is on the search depth for my engine.

      - Let's say one square amplifies 3 'U', allowing user to win the game. Then assume the square directly above it amplifies 3 'C', allowing comp to
        win the game. The square for the comp is MUCH less valuable than the square for the user, and to explain consider the 2 possible scenarios:
		- SCENARIO 1) Comp is forced via zugzwang to move under the user square, allowing the user to win the game.
                - SCENARIO 2) User is forced via zugzwang to move under the user square, allowing comp to fill the user square, and then user fills the
                              comp square. No one wins from the interaction (okay, maybe there is ANOTHER comp square above, but two winning squares
                              directly stacked is handled by the evaluation function).

                You'll have to do some experimentation to see how much less to make the upper square worth than the lower square. Note that...
			- You should probably evaluate the lower square the same, since its value isn't affected whatsoever by the upper square.
                        - However, the upper square should be given a lesser value, since it is IMPOSSIBLE for it to ever be used to win the game (for whichever
                          player is being considered to use it at the moment in the smart evaluation function).
				- Okay, it's not IMPOSSIBLE if the player using the lower square misses they can win in one move, allowing the other player
                                  to fill the square and then fill the upper square, winning the game (so this requires TWO BLUNDERS in a row!).

      - Keep the amplifying vectors constantly sorted. Should be sorted in some fashion so that the lower squares for each
        column are looked at first.
            - When you create a new position object (going 1 depth deeper into calculations), then new amplifying squares you add
              to the amplifying vectors should be placed in sorted order.
            - Insertion sort will be great for this! Since the amplifying vectors are going to be always kept sorted in the aforementioned
              fashion.

            - Note that in the think_ functions (which are called from main()), it is necessarily guaranteed the passed
              in amplifying vector params will be sorted in the desired fashion. So in these think_ functions,
              make sure the comp sorts the passed in amplifying vectors (stl sort works best here, since it should be assumed
              the amplifying vectors aren't at all sorted).

      - A "when push comes to shove" idea for squares making a column a "finished column".
            - For this kind of square, both and user and comp are threatening a 4-in-a-row.
            - But if this square is ever utilized to seize a win, only the comp or user will get to take advantage of it.
            - So, it makes sense to figure out which player will actually be able to use the square, should one of the players
              eventually end up in zugzwang.

            - This can be easily figured out by counting the number of empty squares (not including the winning square and directly above it)
              and factoring in whose turn it is currently.
                 - Example: If it's the comp's turn and there are an odd number of squares, then if the winning square gets used
                   it will happen when the comp has to move in the square under the winning square (due to zugzwang) and the user
                   places on the winning square. So only count the winning square as an advantage for the user!
                        - Still make a column remain a finished column, of course.

            - **IMPORTANT EXCEPTION**: If the square directly under the winning square is an amplifying square into a 4-in-a-row
              for one of the players, then DISCARD THE ABOVE ALGORITHM. Things have completely changed now.

                - If the square directly under COULD become an amplifying square for a 4-in-a-row, then things are a bit tricky:
                    - You could run the above algorithm. Then, for whoever is the "victor" of that algorithm, check if
                      the opponent could possibly make the square directly under a winning square in the future.
                        - If the opponent can, then discard the algorithm.

      - INTERESTING IDEA: I'm guessing that in smart_evaluation(), a temp_board is used to store 'A' chars in squares
              that win for one of the players. Instead, make this temp_board store ENUMS in each square, for the following options:
                - An enum if the square has a 'C'
                - An enum if the square has a 'U'
                - An enum representing what the square does for the comp and user. Possibilites for each of them:
                    - Square could amplifying a 4-in-a-row for a player, or POSSIBLY amplify a 4-in-a-row in the future,
                      or NEVER be able to amplify a 4-in-a-row in the future.
                    - Makes 9 enums, since these 3 possibilities are for each player. Example enum value:
                      "AmplifiesCompAndNothingUser".  Means square wins for comp, and can never do anything for the user.

                - With these enums, it will now be straightforward to check if a square wins for both user and comp (causing finished column
                  and meaning the whole above algorithm runs), as well as straightforward to check the status of whether
                  a square could possibly be a 4-in-a-row for one of the players (useful for the square UNDERNEATH a finished column square,
                  in the above algorithm).

                - NOTE: If making temp_board store enums is too inefficient (either time or space wise), you could continue using chars,
                  as long as you remember what char means what for the 9 possibilities for each empty square.

      - Maybe only clean up the amplifying vectors at something like depth 6 instead of at depth 0 in 2nd constructor?
        Reason is that there are about 7 times more depth 7 positions than depth 6 ones, and for every depth 6 position
        that you clean up, its 7 depth 7 child positions will be evaluated faster.

            - Note that this logic can be continually applied backwards. Maybe it's best to clean up at every depth level?
              Try to mathemtically prove something to help here.


        **NOTE**: The bottom suggestion is probably useless when using iterative deepening, since iterative deepening
        sorts moves from likely best to likely worst, based off hash table giving move orders of positions that were looked at
        at one less depth.

       - Get the possible_moves vector in position.h to store moves in this order: D,E,C,F,B,G,A
         It goes from the middle out, since in general moves in the middle tend to be better. This will allow the computer
         to find better moves quicker, allowing more alpha-beta pruning.
            - Also, STOP randomizing possible_moves' order on each instantiation in constructors 2 & 3! Not efficient!

*/
