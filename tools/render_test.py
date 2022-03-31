import math
import pygame

WHITE = (255, 255, 255)
RED = (255, 26, 1)

def get_lat(phase, i):
    if i == 0:
        return -90.0
    elif i == 9:
        return 90.0
    else:
        return -90.0 + phase + (i-1) * 22.5

def calc_points(phase):
    points = {}
    sin_lat = {}
    for i in range(10):
        points[i] = {}
        lat = get_lat(phase, i)
        sin_lat[i] = math.sin(lat * math.pi / 180.0)

    for j in range(9):
        lon = -90.0 + j * 22.5
        y = math.sin(lon * math.pi / 180.0)
        l = math.cos(lon * math.pi / 180.0)
        for i in range(10):
            x = sin_lat[i] * l
            points[i][j] = (x, y)

    return points

def tilt_sphere(points, ang):
    st = math.sin(ang * math.pi / 180.0)
    ct = math.cos(ang * math.pi / 180.0)
    for i in points:
        for j in points[i]:
            x, y = points[i][j]
            x, y = x * ct - y * st, x * st + y * ct
            points[i][j] = x, y

def scale_and_translate(points, s, tx, ty):
    for i in points:
        for j in points[i]:
            x, y = points[i][j]
            x, y = x * s + tx, y * s + ty
            points[i][j] = x, y

def transform(points, s, tx, ty):
    tilt_sphere(points, 17.0)
    scale_and_translate(points, s, tx, ty)

def fill_tiles(surface, points, alter):
    for j in range(8):
        for i in range(9):
            p1 = points[i][j]
            p2 = points[i+1][j]
            p3 = points[i+1][j+1]
            p4 = points[i][j+1]
            pygame.draw.polygon(surface, RED if alter else WHITE, (p1, p2, p3, p4))
            alter = not alter

def calc_and_draw(surface, phase, scale, x, y):
    points = calc_points(phase % 22.5)
    transform(points, scale, x, y)
    fill_tiles(surface, points, phase >= 22.5)

def init_and_run_loop():
    pygame.init()

    ball_size = 32

    # render buffer
    buffer_size = (ball_size, ball_size)
    buffer = pygame.Surface(buffer_size)

    screen = None # disable on-screen display
    #screen_size = (640, 480)
    #screen = pygame.display.set_mode(screen_size)

    scale = ball_size / 2.0
    x = int(scale)
    y = int(scale)

    phase_step = 7.5
    for frame in range(6):
        phase = (frame * phase_step) % 45.0
        print(f"{frame} - {phase}")

        calc_and_draw(buffer, phase, scale, x, y)
        pygame.image.save(buffer, f"ball{frame}.png")

        if screen:
            screen.blit(buffer, (0, 0))
            pygame.display.flip()


    pygame.quit()

if __name__ == '__main__':
    init_and_run_loop()
