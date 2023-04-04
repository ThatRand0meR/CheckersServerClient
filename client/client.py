# Import packages
import pygame
import math
import time
import os
import socket
from threading import Thread, Event
from queue import Queue
from sys import exit

# Inititalize Pieces
empty = 0
# For the first move, black's pieces are considered friendly
black = {"pawn": 1, "king": 3}
white = {"pawn": 2, "king": 4}

# Initalize board size
rows = 8
columns = 8


# Define colours
class Colours:
    square_black = (0, 0, 0)
    square_white = (255, 255, 255)

    piece_white = (209, 45, 45)
    piece_black = (87, 51, 51)
    king_cross = (255, 255, 255)


# This sets the width, height and margin of each board cell
window_size = [500, 500]
window_width = window_size[0]
window_height = window_size[1] - window_size[0] // 20
total_rows = 8
total_columns = 8
width = (window_width // total_columns)
height = (window_height // total_rows)

# Set the radius and border border of each checker piece
radius = (window_width // 20)
border = (window_width // 200)


# Create board
def create_board():
    board = [[empty for column in range(columns)] for row in range(rows)]
    place_starting_pieces(board)
    return board


def place_starting_pieces(board):
    """Assign starting checker pieces for white and black"""
    # Assign starting board locations for black
    for current_row in range(5, 8, 2):
        for current_column in range(0, 8, 2):
            board[current_row][current_column] = black['pawn']
    for current_row in range(6, 7):
        for current_column in range(1, 8, 2):
            board[current_row][current_column] = black['pawn']

    # Assign starting board locations for white
    for current_row in range(0, 3, 2):
        for current_column in range(1, 8, 2):
            board[current_row][current_column] = white['pawn']
    for current_row in range(1, 2):
        for current_column in range(0, 8, 2):
            board[current_row][current_column] = white['pawn']


def draw_board(screen, board, width, height, radius, border):
    for row in range(8):
        for column in range(8):
            # Draw all grid locations as either white or black rectangle
            if (row + column) % 2 == 0:
                colour = Colours.square_white
            else:
                colour = Colours.square_black
            rect = pygame.draw.rect(screen, colour, [width * column, height * row, width, height])

            myfont = pygame.font.SysFont('Arial', int(10))
            textsurface = myfont.render(str(get_cell_no(column, row)), False, (255, 255, 255))
            screen.blit(textsurface, (width * column, height * row))

            rect_center = rect.center
            # Draw black pieces
            if board[row][column] == 1:
                pygame.draw.circle(screen, Colours.piece_black, rect_center, radius)
            # Draw white pieces
            if board[row][column] == 2:
                pygame.draw.circle(screen, Colours.piece_white, rect_center, radius)
            # Drawing king pieces borders
            if board[row][column] == 3:
                pygame.draw.circle(screen, Colours.piece_black, rect_center, radius)
                pygame.draw.circle(screen, Colours.king_cross, rect_center, radius // 5, 0)
            if board[row][column] == 4:
                pygame.draw.circle(screen, Colours.piece_white, rect_center, radius)
                pygame.draw.circle(screen, Colours.king_cross, rect_center, radius // 5, 0)


def draw_popup(screen, message, colour, error):
    rect = pygame.Rect(0, window_height, window_width, window_height // 5)
    pygame.draw.rect(screen, colour, rect, 0)

    font_size = 15
    if error is True:
        font_size /= 1.5

    myfont = pygame.font.SysFont('Arial', int(font_size))
    textsurface = myfont.render(message, False, (0, 0, 0))
    if error is True:
        screen.blit(textsurface, (0, window_height))
    else:
        screen.blit(textsurface, (window_width // 2 - 120, window_height))


def get_cell_coordinates(cell_no):
    cell_no = int(cell_no)

    y = (cell_no // 4)
    x = 2 * ((cell_no - 1) % 4)

    if cell_no % 4 == 0:
        y = y - 1

    if y % 2 == 0:
        x = x + 1

    return [y, x]


def get_cell_no(x, y):
    if x % 2 == 0 and y % 2 == 1:
        return math.ceil(x / 2) + (y * 4) + 1

    elif x % 2 == 1 and y % 2 == 0:
        return math.ceil(x / 2) + (y * 4)

    # White cell
    else:
        return 0


def check_notation(move):
    if move == "":
        return False

    list = move.split()
    if len(list) != 3:
        return False

    cell_from = list[0]
    move_type = list[1]
    cell_to = list[2]

    if int(cell_from) <= 0 or int(cell_from) > 32: 
        return False

    if int(cell_to) <= 0 or int(cell_to) > 32: 
        return False

    if move_type != '-' and move_type != 'x':
        return False

    return True


def move_piece(screen, board, cell_from, cell_to, became_queen):
    xyfrom = get_cell_coordinates(cell_from)
    xyto = get_cell_coordinates(cell_to)

    piece = board[xyfrom[0]][xyfrom[1]]
    if became_queen is not None and piece <= 2:
        piece += 2

    board[xyfrom[0]][xyfrom[1]] = empty
    board[xyto[0]][xyto[1]] = piece

    draw_board(screen, board, width, height, radius, border)
    pygame.display.flip()


def remove_piece(screen, board, cell):
    if cell == 0:
        return
    else:
        xycaptured = get_cell_coordinates(cell)
        board[xycaptured[0]][xycaptured[1]] = empty

    draw_board(screen, board, width, height, radius, border)
    pygame.display.flip()


def perform_move(move, screen, board):
    cell_from = move[0]
    move_type = move[1]
    cell_to = move[2]

    if move_type == '-':
        move_piece(screen, board, cell_from, cell_to, None)

    if move_type == 'x':
        move_piece(screen, board, cell_from, cell_to, None)
        #remove_piece(screen, board, ?)


def read_from_server(socket):
    return socket.recv(512).decode("ascii")


def write_to_server(socket, message):
    socket.send(bytes(message, "ascii"))

### GAME

pygame_message_queue = Queue()
board = create_board()
t_event = Event()


def game_loop(s):
    while True:
        if t_event.is_set():
            raise Exception("Connection thread terminated")

        data = read_from_server(s)
        print(str("SERVER: ") + str(data))

        if data == "YOUR_TURN":
            pygame_message_queue.put("DRAW_POPUP - Your turn")
            message = input("What's your next move?: ")

            while check_notation(message) is False:
                message = input("Your move notation is incorrect - try another one: ")

            player_move = message
            write_to_server(s, message)
            is_legal_move = read_from_server(s)
            while is_legal_move.startswith("NOT_LEGAL"):
                message = input("Your move is illegal - try another one: ") 
                player_move = message
                write_to_server(s, message)
                is_legal_move = read_from_server(s)

            pygame_message_queue.put("MOVE " + player_move)

        elif data == "OPPONENTS_TURN":
            pygame_message_queue.put("DRAW_POPUP - Opponent's turn")
            opponents_move = read_from_server(s)
            pygame_message_queue.put("MOVE " + opponents_move)

        elif data == "EXIT":
            return 0


def main():
    ### Connecting to server
    ip = input('Please enter server address [default: localhost]: ')
    port = 1234

    if ip == "":
        ip = "localhost"

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, port))
    s.send(bytes("WANNA_PLAY", "ascii"))
    data = read_from_server(s)

    player_color = ""

    if data == "GAME_START_WHITE":
        print("SERVER: You are playing white pieces")
        player_color = "white"
    elif data == "GAME_START_BLACK":
        print("SERVER: You are playing black pieces")
        player_color = "black"
    else:
        exit(1)

    # Start communication thread
    t1 = Thread(target=game_loop, args=(s,))
    t1.daemon = True
    t1.start()

    # Initalize pygame
    pygame.init()
    pygame.font.init()

    screen = pygame.display.set_mode(window_size)
    pygame.display.set_caption("Checkers: " + player_color + " player")
    Running = True

    while Running is True:
        # Game event handling
        if pygame_message_queue.qsize() > 0:
            message = str(pygame_message_queue.get())
            if message.startswith("DRAW_POPUP"):
                draw_popup(screen, message, (50, 150, 255), error=False)
            elif message.startswith("MOVE"):
                move = message.split()
                move.pop(0)
                perform_move(move, screen, board)

        # Pygame event handling
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                Running = False

        draw_board(screen, board, width, height, radius, border)
        pygame.display.update()

    pygame.quit()
    exit(0)


if __name__ == '__main__':
    main()
