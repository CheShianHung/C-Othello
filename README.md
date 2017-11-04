# C-Othello
This program allows the user to play Othello with computer implemented with Alpha-Beta pruning algorithm.  
  
  
In this program, the user has to play as white and the ai will play as black. In each game, the user has to move first. The bright blue bot in each block is used to indicated the possible moves for the user. The total white number and the total black number are recorded in the right hand side. The user can always start another game by simply clicking the restart button at the bottom right cornor. The evaluation function for the alpha-beta search algorithm is calculated by the move positions in different perspective and the difference of the number of black and white circles. For instance, in the getMoveValue function, the values for the outter four corners are set to 70, meaning that if we predict the player makes a move at one of the cornor, the value for the node will be added with -70, but if it is the ai does the cornor, the value will be added with 70. This behavior can be observed in the expendNode function. Based on the observation, with this evaluation function and searching implementation, the ai will be the most difficult when the depth limit is 2. Any value more than 2 seems to overkill the selection. The evaluated values for many future option can easily confuse the algorithm, causing the bad move for the current state to be selected. The future option can be implementing more on the node sselection.

# Screen Captures
![alt text](https://user-images.githubusercontent.com/17507896/32405883-5661f4c8-c144-11e7-8137-1582cb7486f7.png)
![alt text](https://user-images.githubusercontent.com/17507896/32405884-577781e8-c144-11e7-86df-ba2d8a583482.png)
![alt text](https://user-images.githubusercontent.com/17507896/32405885-586aa288-c144-11e7-9935-2ec45eccf54e.png)
![alt text](https://user-images.githubusercontent.com/17507896/32405886-594322e8-c144-11e7-8578-a10ad95a1aab.png)
