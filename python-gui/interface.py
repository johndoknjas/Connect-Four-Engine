# Important:
    # When you update the C++ program, and want to run the .exe form of this
    # Python program (interface.exe):
        # First, you'll have to type in cmd, in the directory /bin/Debug:
        # "pyinstaller interface.py".
        
        # Then, before running the updated interface.exe in /dist/interface,
        # make sure to copy the files in /bin/Debug into that directory.
        # These are: directory.txt, error_effect.wav, move_effect.wav,
        # OngoingScore.txt, second_move_effect.wav, and Version 1 - April 25, 2018.exe.
            # Database_C.db is stored somewhere else, such as in
            # Fun Coding Projects\Connect Four, and is referenced via an absolute path
            # in directory.txt.

# Rules for threading:
    # If any of the threads (including the main thread) want the whole program to crash,
    # then set kill_daemon to true before doing os._exit(). 
    # This will notify the other threads to crash as well.


# To do:
    
    # For the reading from file functions (for both C++ and Python), maybe
    # add a small delay (like 0.01 s) for each iteration of the while loop.
        # The point is to minimize the computational power, should the C++
        # engine be thinking at that time.
        
        # Think of any other ways the engine might be losing some of its
        # computational power to other processes.
            # In the while loop of pygame_stuff, it looks like not much
            # computational power if being used. The visual display is only
            # updated when something changes/happens, so it's not continuous.

# Bugs:

    # Possible bug: python program sometimes crashes when thinking_time is
    # really low (like 0.0001 s). Might be inherent to the C++ engine,
    # and if this is the case then it's fine.
        # Note that I wrote this comment a while back, so it could be
        # non-applicable now (if it ever was an issue in the first place).



import subprocess
import time
from pathlib import Path
import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import threading
import sys
import pygame
#from pygame.locals import *
import glob

starting_num = 1587992
    
filename = str(starting_num) + ".txt"

kill_daemon = False

lowercase_list = ["a", "b", "c", "d", "e", "f", "g"]

uppercase_list = ["A", "B", "C", "D", "E", "F", "G"]

is_user_turn = False # stores true whenever it is the user's turn in the game.

pieces_to_display = []
has_pieces_to_display_been_updated = False

updating_pygame_display = False

enable_GUI_buttons = False

user_chosen_move = -1

data_currently_being_gathered = 0
    # 0: Getting the user to decide whether to play only the starting position.
        # This is only the case once in the whole run of the app.
    # 1: Getting the user to decide who goes first.
    # 2: Getting the user to decide whether to play red/yellow.
    # 3: Getting the user to decide whether to play again or not.
    # -1: A game is currently being played, no data being gathered.
    
    # The sequence this variable will take, if multiple games are played, is:
    
    # 0 --> 1 --> 2 --> -1 --> 3 --> *repeat cycle at the 1*.

# Boolean variables, for pregame binary decisions from the user:
play_only_starting_pos = None
user_goes_first = None
user_plays_red = None
play_another_game = None

who_just_won = None
    # 0: draw
    # 1: comp won
    # 2: user won

num_games_comp_won = None
num_games_user_won = None
num_games_drawn = None

production_mode = True

def my_console_print(message):
    if not(production_mode):
        print(message)

def convert_each_string_to_ints(lst):
    result = []
    
    for current_string in lst:
        value = int(current_string)
        result.append(value)
    
    return result

# Function also updates starting_num and filename.
def read_from_textfile(should_just_get_int):
    global starting_num
    global filename
    
    # Note - the "should_just_get_int" parameter stores true if this function
    # should just be returning an int, and false if the function should
    # return a whole list (such as when the C++ program sends a file with
    # 42 ints representing the piece to display for each square).
    
    while (True):
        # See if the textfile even exists yet - the C++ program may have not created it.
        
        file = Path(filename)
        
        if (file.is_file()):
            file_contents = open(filename, "r")
            lines = file_contents.readlines()
            if (len(lines) > 0):
                # But before doing this, increment the filename and starting_name.
                starting_num += 1
                filename = str(starting_num) + ".txt"
                file_contents.close()
                
                # Now to either return the first element/int of lines, or
                # to return the whole list. Depends on the function parameter:
                
                if (should_just_get_int):
                    return int(lines[0])
                else:
                    return convert_each_string_to_ints(lines)
                
            file_contents.close()

# Function updates starting_num and filename when done.
def write_int_to_textfile(value):
    global starting_num
    global filename
    
    file = open(filename, "w")
    
    file.write(str(value))
    
    file.close()

    starting_num += 1
    filename = str(starting_num) + ".txt"

def crash_program_if_subprocess_quits(p):
    global kill_daemon
    
    while (True):
        time.sleep(1.0)
        if (kill_daemon):
            return
        if (p.poll() != None):
            # subprocess has crashed, so end the entire Python program.
            # Since this function is just called in a separate thread, use:
            kill_daemon = True # to get the other daemon thread(s) to also end.
            os._exit(0)
    
def display_board():
    global pieces_to_display
    global has_pieces_to_display_been_updated
    
    # This function will no longer be used, since a GUI board will replaced
    # the text board.
    
    if (len(pieces_to_display) != 42):
        raise Exception("pieces_to_display only has these many elements:", len(pieces_to_display))
    
    print()
    
    print(" A   B   C   D   E   F   G\n")
    
    current_index = 0
    
    for r in range (6):
        print(" ", end = "")
        for c in range (7):            
            if (pieces_to_display[current_index] == 0):
                print (" ", end = "")
            elif (pieces_to_display[current_index] == 1):
                print("X", end = "")
            else:
                print("O", end = "")
            print(" | ", end = "")
            
            current_index += 1
        print()
        print("----------------------------")
    
def is_valid_letter(letter):
    return ((letter in lowercase_list) or (letter in uppercase_list))

# Pre-condition: letter is a valid letter.
def convert_letter_to_index(letter):
    if (letter in lowercase_list):
        return (lowercase_list.index(letter))
    elif (letter in uppercase_list):
        return (uppercase_list.index(letter))
    else:
        sys.exit() # Raising an error.

def create_signal_file():
    file = open("signal.txt", "w")
    file.close()

def delete_signal_file():
    # Deletes the signal.txt file from the previous game.
    # It is important to do this, or else the C++ program will quit,
    # once it sees this file exists.
    
    if os.path.exists("signal.txt"):
        os.remove("signal.txt")

def is_column_legal(column_index):
    global pieces_to_display
    
    # use the pieces_to_display list to check this.
    
    if column_index < 0 or column_index > 6:
        return False
    
    current_iteration = 0
    
    for r in range(6):
        for c in range(7):
            if r == 0 and c == column_index:
                # return True or False here, depending on if this square
                # is empty.
                
                return pieces_to_display[current_iteration] == 0
            
            current_iteration += 1
    
    raise Exception("Shouldn't have reached the end of this function.")

def quit_procedure_from_pygame():
    global kill_daemon
    
    kill_daemon = True # gets the other daemon thread to finish too.
    pygame.quit()
    create_signal_file() # will tell the C++ program to crash as well.
    os._exit(0) # Should get this thread here, as well as the main thread,
                # to finish.

def update_pygame_board(line_width_plus_radius, circle_radius, screen):
    global pieces_to_display
    
    current_iteration = 0
    for r in range(6):
        for c in range(7):
            x_coor = 0
            y_coor = 0
            
            x_coor = (c+1) * line_width_plus_radius
            x_coor += c * circle_radius
            
            y_coor = (r+1) * line_width_plus_radius
            y_coor += r * circle_radius
            
            rgb_values = (210,210,210)
            
            if (pieces_to_display[current_iteration] == 1):
                rgb_values = (255,51,51)
            elif (pieces_to_display[current_iteration] == 2):
                rgb_values = (225,225,0)
            
            pygame.draw.circle(screen, rgb_values, (x_coor, y_coor), 
                               circle_radius, 0)
            
            current_iteration += 1
    
    pygame.display.update()

def cover_over_text(screen, font, coordinate):

    blank_space = ' ' * 99
    
    text = font.render(blank_space, True, (0,0,255), (0,0,255))
            
    textRect = text.get_rect()
    
    textRect.center = coordinate
    
    screen.blit(text, textRect)
    
    pygame.display.update()

def display_text(message, central_coordinate, screen, font):
    text = font.render(message, True, (255,255,255), (0,0,255))
    
    textRect = text.get_rect()
    
    textRect.center = central_coordinate
    # Note that 465 is half the screen's width and 400 is half the height.
    
    screen.blit(text, textRect)
    
    pygame.display.update()
    
def pygame_stuff(placeholder):
    global kill_daemon
    global pieces_to_display
    global has_pieces_to_display_been_updated
    global updating_pygame_display
    global enable_GUI_buttons
    global user_chosen_move
    
    global data_currently_being_gathered
    global play_only_starting_pos
    global user_goes_first
    global user_plays_red
    global play_another_game
    
    global who_just_won
    
    global num_games_comp_won
    global num_games_user_won
    global num_games_drawn
    
    pygame.mixer.init()
    pygame.init()
    move_sound_effect = pygame.mixer.Sound("move_effect.wav")
    error_sound_effect = pygame.mixer.Sound("error_effect.wav")
    # mixer.music.set_volume(0.5)
    
    font = pygame.font.SysFont('timesnewroman', 35)
    
    factor = 1.24
    
    screen = pygame.display.set_mode((round(750*factor), round(645*factor)))

    pygame.display.set_caption("Connect Four AI")
    
    screen.fill((0, 0, 255))
    
    line_width_plus_radius = round(60 * factor)
    circle_radius = round(45 * factor)
    
    minimized_line_width_plus_radius = round(55 * factor)
    minimized_circle_radius = round(41 * factor)
    
    #normal_button_width = 119.5714286
    button_height = 799.8
    
    buttons = []
    
    for i in range(7):
        buttons.append(pygame.Rect((i+1)*line_width_plus_radius + (i-1)*circle_radius, 
                                   0, 2*circle_radius, button_height))
            
    #for r in range(6):
    #    for c in range(7):
    #        x_coor = 0
    #        y_coor = 0
            
    #        x_coor = (c+1) * line_width_plus_radius
    #        x_coor += c * circle_radius
            
    #        y_coor = (r+1) * line_width_plus_radius
    #        y_coor += r * circle_radius
            
    #        pygame.draw.circle(screen, (210,210,210), (x_coor, y_coor), circle_radius, 0)
            
    pygame.display.update()
    
    # has_screen_been_filled_once_during_game = False
    # is_screen_expanded = True
    
    just_minimized_pygame_board = False
    just_maximized_pygame_board = False
    
    displayed_text_prompt_data_1 = False
    displayed_text_prompt_data_2 = False
    displayed_text_prompt_data_3 = False
    
    displaying_starting_position = True
    
    about_to_quit = False
    
    on_first_game = True
    
    while True:
        
        if (kill_daemon):
            pygame.quit()
            return
        
        if data_currently_being_gathered == 0:
            
            while num_games_comp_won == None or num_games_user_won == None or num_games_drawn == None:
                pass
            
            display_text('Press s/r to play the starting position or a random fair position.',
                         (465,160), screen, font)
            
            display_text('Ongoing score from games beginning at a random position:', 
                         (465,325), screen, font)
            
            display_text('The computer has won ' + str(num_games_comp_won) + ' games.',
                         (465,375), screen, font)
        
            display_text('You have won ' + str(num_games_user_won) + ' games.',
                         (465,425), screen, font)
            
            display_text('There have been ' + str(num_games_drawn) + ' draws.',
                         (465,475), screen, font)
        
            display_text('To toggle between sound effects for moves, press 1 and 2.',
                         (465,640), screen, font)
            
            #text = font.render('Press s/r to play the starting position or a random fair position.', 
            #                   True, (255,255,255), (0,0,255))
            
            #textRect = text.get_rect()
            
            #textRect.center = (465, 400)
            # Note that 465 is half the screen's width and 400 is half the height.
            
            #screen.blit(text, textRect)
            
            #pygame.display.update()
            
            #screen = pygame.display.set_mode((round(1200*factor), round(645*factor)))
            #screen.fill((0, 0, 255))
            #pygame.display.update()
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    quit_procedure_from_pygame()
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_s:
                        play_only_starting_pos = True
                        data_currently_being_gathered = 1
                    elif event.key == pygame.K_r:
                        play_only_starting_pos = False
                        data_currently_being_gathered = 1
                    elif event.key == pygame.K_2:
                        move_sound_effect = pygame.mixer.Sound("second_move_effect.wav")
                    elif event.key == pygame.K_1:
                        move_sound_effect = pygame.mixer.Sound("move_effect.wav")
        
        elif data_currently_being_gathered == 1 and not(about_to_quit):
            
            who_just_won = None
            
            displayed_text_prompt_data_3 = False
            
            if not(displayed_text_prompt_data_1):
                
                if on_first_game:
                    cover_over_text(screen, font, (465,160))
                    
                    display_text('Press f/s to go first or second.', (465,160), screen, font)
                else:
                    cover_over_text(screen, font, (465,755))
                    
                    display_text('Press f/s to go first or second.', (465,755), screen, font)
                
                #text = font.render('Press f/s to go first or second.', 
                #                   True, (255,255,255), (0,0,255))
                
                #textRect = text.get_rect()
                
                #textRect.center = (465, 755)
                
                #screen.blit(text, textRect)
                
                #pygame.display.update()
                
                displayed_text_prompt_data_1 = True
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    quit_procedure_from_pygame()
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_f:
                        user_goes_first = True
                        data_currently_being_gathered = 2
                    elif event.key == pygame.K_s:
                        user_goes_first = False
                        data_currently_being_gathered = 2
                    elif event.key == pygame.K_2:
                        move_sound_effect = pygame.mixer.Sound("second_move_effect.wav")
                    elif event.key == pygame.K_1:
                        move_sound_effect = pygame.mixer.Sound("move_effect.wav")
                        
        elif data_currently_being_gathered == 2:
            
            displaying_starting_position = True # for the game that's about to start.
            
            displayed_text_prompt_data_1 = False
            
            if not(displayed_text_prompt_data_2):
                
                if on_first_game:
                    cover_over_text(screen, font, (465,160))
                    
                    display_text('Press r/y to play red or yellow.', (465,160), screen, font)
                else:
                    cover_over_text(screen, font, (465,755))
                    
                    display_text('Press r/y to play red or yellow.', (465,755), screen, font)
                
                #text = font.render('Press r/y to play red or yellow.', 
                #                   True, (255,255,255), (0,0,255))
                
                #textRect = text.get_rect()
                
                #textRect.center = (465, 755)
                
                #screen.blit(text, textRect)
                
                #pygame.display.update()
                
                displayed_text_prompt_data_2 = True
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    quit_procedure_from_pygame()
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_r:
                        user_plays_red = True
                        data_currently_being_gathered = -1
                        # has_screen_been_filled_once_during_game = False
                    elif event.key == pygame.K_y:
                        user_plays_red = False
                        data_currently_being_gathered = -1
                        # has_screen_been_filled_once_during_game = False
                    elif event.key == pygame.K_2:
                        move_sound_effect = pygame.mixer.Sound("second_move_effect.wav")
                    elif event.key == pygame.K_1:
                        move_sound_effect = pygame.mixer.Sound("move_effect.wav")
        
        elif data_currently_being_gathered == 3:
            
            #if not(is_screen_expanded):
            #    screen = pygame.display.set_mode((round(1200*factor), round(645*factor)))
            #    screen.fill((0, 0, 255))
            #    pygame.display.update()
            #    is_screen_expanded = True
            
            #print("True" if displayed_text_prompt_data_3 else "False")
            
            just_maximized_pygame_board = False
            
            if not(just_minimized_pygame_board):
                screen.fill((0, 0, 255))
                pygame.display.update()
                just_minimized_pygame_board = True
                update_pygame_board(minimized_line_width_plus_radius, minimized_circle_radius, screen)
            
            if not(displayed_text_prompt_data_3):
            
                cover_over_text(screen, font, (465,755))
                
                while who_just_won == None:
                    pass
                
                if who_just_won == 1:
                    display_text('The computer won.', (465,755), screen, font)
                elif who_just_won == 2:
                    display_text('You won!', (465,755), screen, font)
                else:
                    display_text('The game was a draw.', (465,755), screen, font)
                
                time.sleep(1.33)
                
                cover_over_text(screen, font, (465,755))
                
                display_text('Press y/n to play again or exit the program.', (465,755), screen, font)
                
                #text = font.render('Press y/n to play again or exit the program.', 
                #                   True, (255,255,255), (0,0,255))
                
                #textRect = text.get_rect()
                
                #textRect.center = (465, 755)
                
                #screen.blit(text, textRect)
                
                #pygame.display.update()
                
                displayed_text_prompt_data_3 = True
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    quit_procedure_from_pygame()
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_y:
                        play_another_game = True
                        on_first_game = False
                        data_currently_being_gathered = 1
                    elif event.key == pygame.K_n:
                        play_another_game = False
                        about_to_quit = True
                        data_currently_being_gathered = 1
                    elif event.key == pygame.K_2:
                        move_sound_effect = pygame.mixer.Sound("second_move_effect.wav")
                    elif event.key == pygame.K_1:
                        move_sound_effect = pygame.mixer.Sound("move_effect.wav")
        
        else: # data_currently_being_gathered = -1; the game is being played.
            
          #  if (not(has_screen_been_filled_once_during_game)):
          #      screen.fill((0, 0, 255))
          #      has_screen_been_filled_once_during_game = True
          #      pygame.display.update()
            
            #if is_screen_expanded:
            #    screen = pygame.display.set_mode((round(750*factor), round(645*factor)))
            #    screen.fill((0, 0, 255))
            #    pygame.display.update()                
            #    is_screen_expanded = False
            
            displayed_text_prompt_data_2 = False
            
            just_minimized_pygame_board = False
            
            if not(just_maximized_pygame_board):
                screen.fill((0, 0, 255))
            #    pygame.display.update()
                just_maximized_pygame_board = True
        
            if (has_pieces_to_display_been_updated):
                
                my_console_print("pieces to display has been updated!")
                
                if not(displaying_starting_position) and is_user_turn:
                    # user's turn now, meaning the computer just moved.
                    # So play the sound effect for it.
                    move_sound_effect.play()
                
                has_pieces_to_display_been_updated = False
                updating_pygame_display = True
                # For some circles (where there are pieces), make them
                # red or yellow.
                
                # use int values in pieces_to_display to display red, yellow, empty.
                
                update_pygame_board(line_width_plus_radius, circle_radius, screen)
                
                updating_pygame_display = False
                
                pygame.display.update()
                
                displaying_starting_position = False
                
            for event in pygame.event.get():
                # Check if the close button has been pressed (this is one of the events):
                if event.type == pygame.QUIT:
                    # The user has closed the pygame window. Therefore, end the whole
                    # program (i.e., this thread, the main thread, and any other
                    # daemon thread(s)).
                    
                    quit_procedure_from_pygame()
                    
                    # kill_daemon = True # gets the other daemon thread to finish too.
                    
                    #pygame.quit()
                    
                    #create_signal_file() # will tell the C++ program to crash as well.
                    
                    #os._exit(0) # Should get this thread here, as well as the main thread,
                                # to finish.
                
                #elif event.type == VIDEORESIZE:
                
                elif event.type == pygame.MOUSEBUTTONDOWN and enable_GUI_buttons and is_user_turn:                
                    index_of_column_chosen = None
                    
                    mouse_pos = event.pos
                    
                    if len(buttons) != 7:
                        raise Exception("The size of the buttons list isn't 7.")
                    
                    for i in range(len(buttons)):
                        if (buttons[i].collidepoint(mouse_pos)):
                            # So, the user clicked on buttons[i].
                            index_of_column_chosen = i
                            break
                    
                    # Check if this move is valid:
                    
                    if index_of_column_chosen != None and is_column_legal(index_of_column_chosen):
                        # It is legal - proceed with the move.
                        # CONTINUE HERE.
                        # Also, don't get the C++ to send data back, saying whether
                        # the move is legal - don't need that.
                        
                        move_sound_effect.play()
                        
                        user_chosen_move = index_of_column_chosen
                        
                        time.sleep(0.05)
                        
                        # write_int_to_textfile(index_of_column_chosen)
                    
                    else:
                        error_sound_effect.play()
                        
                        my_console_print("You cannot move into that column - please try again.")
                
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_2:
                        move_sound_effect = pygame.mixer.Sound("second_move_effect.wav")
                    elif event.key == pygame.K_1:
                        move_sound_effect = pygame.mixer.Sound("move_effect.wav")
                    
                # Add more events, for buttons here...

def main():
    
    global kill_daemon
    global is_user_turn
    global pieces_to_display
    global has_pieces_to_display_been_updated
    global enable_GUI_buttons
    global user_chosen_move
    
    global data_currently_being_gathered
    global play_only_starting_pos
    global user_goes_first
    global user_plays_red
    global play_another_game
    
    global who_just_won
    
    global num_games_comp_won
    global num_games_user_won
    global num_games_drawn
    
    # p = subprocess.Popen([r"Version 1 - April 25, 2018.exe", "Python"])
    # Assigning the subprocess to var p in order to check if it's still running at any time,
    # using p.poll().
    
    # t1 = threading.Thread(target=crash_program_if_subprocess_quits, args=(p,))
    
    delete_signal_file()
    
    time.sleep(0.05)
    
    for current_file in glob.glob("./Crash report file at time*.txt"):
        os.remove(current_file)
    
    subprocess.Popen([r"a.exe", "Python"])
    
    pygame_thread = threading.Thread(target=pygame_stuff, args=(True,))
    
    pygame_thread.start()
    
    time.sleep(1.3) # to give the C++ program time to delete all the numbered textfiles.
    
    # I need the number of games the computer has won up to this point.
    # Get it from the textfile the C++ program has written to, containing this int.
    
    num_games_comp_won = read_from_textfile(True)
    
    num_games_user_won = read_from_textfile(True)
    
    num_games_drawn = read_from_textfile(True)
    
    my_console_print("Current ongoing match score:\nComp has won " + str(num_games_comp_won) +
                     " games\nYou have won " + str(num_games_user_won) + " games\nThere have been " +
                     str(num_games_drawn) + " draws\n")
    
    # thinking_time = float(input("Enter approximately how long you want the Engine to think on each move: "))
    thinking_time = 1.0
    
    # thinking_time is a floating point value storing the number of seconds for the thinking time.
    # Convert it to an int form storing the number of milliseconds.
    
    milliseconds_thinking_time = int(thinking_time * 1000)
    
    write_int_to_textfile(milliseconds_thinking_time)
    
    my_console_print("Press s/r to play the starting position or a random fair position.")

    # play_starting_position = input("Enter s to play the starting position: ")
    
    while play_only_starting_pos == None:
        pass
    
    if (play_only_starting_pos):
        write_int_to_textfile(1)
    else:
        write_int_to_textfile(0)
    
    while True: # This loop allows the user to play multiple games.
        
        # Boolean global vars, for binary pregame user decisions:
        user_goes_first = None
        user_plays_red = None
        play_another_game = None
    
        # Now figure out if this is the current game. Get this info from the C++ program.
        
        is_first_game = read_from_textfile(True)
        
        # pieces_to_display = []
        has_pieces_to_display_been_updated = False
        updating_pygame_display = False
        
        if (is_first_game != 1):
            # 1 would mean it is the first game. So since it isn't, ask the user
            # if they want to play again:
            
            data_currently_being_gathered = 3
            
            my_console_print("Press y/n to play again or exit the program.")
            
            # play_again = input("Enter y to play again:")
            
            while play_another_game == None:
                pass
            
            if (play_another_game):
                write_int_to_textfile(1)
            else:
                write_int_to_textfile(0)
            
            if not(play_another_game):
                # End the python program (the C++ program will be ending itself now as well).
                
                kill_daemon = True
                
                os._exit(0) # renders the above line kind of useless, since this
                            # should destroy the daemon as well.
                
                # Here it isn't necessary to create the signal.txt file, since the
                # C++ program knows to stop (since 0 was just written to the textfile
                # that it was expecting - this will tell it to naturally end).
                
        my_console_print("Press f/s to go first or second.")
        
        #who_should_go_first = input("Enter y to go first: ")
        
        while user_goes_first == None:
            pass
        
        if (user_goes_first):
            write_int_to_textfile(1)
            is_user_turn = True
        else:
            write_int_to_textfile(0)
            is_user_turn = False
            
        my_console_print("Press r/y to play red or yellow.")
        
        while user_plays_red == None:
            pass
        
        # should_user_play_x = input("Enter X to play red; otherwise, enter O (or any other input) to play yellow: ")
        
        if (user_plays_red):
            write_int_to_textfile(1)
        else:
            write_int_to_textfile(0)
        
        pieces_to_display = read_from_textfile(False)
        has_pieces_to_display_been_updated = True
        time.sleep(0.05) # to get the pygame thread to update the board,
                         # and then reset "has_pieces_to_display..." to False.
        while (updating_pygame_display):
            pass
        
        if not(production_mode):
            display_board()
        
        #display_GUI_board(pieces_to_display)
        
        is_game_over = False
        
        # Game loop:
        
        enable_GUI_buttons = True
        
        game_just_started = True
        
        while (not(is_game_over)):
            if is_user_turn:
                # print("Enter the column to move into:")
                
                while user_chosen_move == -1: # the user is selecting a move in the pygame function.
                    pass
                
                write_int_to_textfile(user_chosen_move)
                
                user_chosen_move = -1            
            
            elif game_just_started:
                # The game just began and it is the comp's turn. So, before
                # updating pieces_to_display with its move (which will then
                # update the pygame board), I want to want a small amount 
                # of time for the user to see the starting position.
                
                time.sleep(0.2)
            
            # Get the updated list of all the pieces in the board:
            
            pieces_to_display = read_from_textfile(False)
            is_user_turn = not(is_user_turn)
            has_pieces_to_display_been_updated = True
            time.sleep(0.05) # not sure what the purpose of this is?
            if not(production_mode):
                display_board()
            is_game_over = read_from_textfile(True)
            game_just_started = False
        
        enable_GUI_buttons = False
        
        my_console_print("The game is over.")
            
        game_result = read_from_textfile(True)
        
        if (game_result == 1):
            who_just_won = 1
            my_console_print("The computer won.")
        elif (game_result == 2):
            who_just_won = 2          
            my_console_print("You won!")
        else:
            who_just_won = 0
            my_console_print("The game was a draw.")
        
        # At this point, control iterates again on the big while loop, allowing the
        # user to play another game if they want to.    
    
    # Now we're at the end of main(), so set the flag to kill the daemon thread:
    
    kill_daemon = True    
    
    # pygame.quit()
    
    create_signal_file() 
    # temporary measure to need this line (although it's fine to keep it in).
    # When you get the game loop set up here in Python, the
    # C++ program should know to naturally stop if the user doesn't want
    # to play again by not entering a 1.
    
if __name__ == '__main__':
    main()
    