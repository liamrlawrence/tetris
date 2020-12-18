//======================================================================================================================
// File Name    : tetris.h
// Description  : Implementation of tetris using ncurses, accessed through a global struct `tetris`
// Authors      : Liam Lawrence
// Created      : December 17, 2020
// License      : MIT License
// Copyright    : (c) 2020, Liam Lawrence
//======================================================================================================================

#ifndef TETRIS_TETRIS_H
#define TETRIS_TETRIS_H

#include <ncurses.h>

typedef struct{
    struct {
        WINDOW *playfield;
        WINDOW *scoreboard;
        WINDOW *nextp;
    } windows;
    void (*init)(void);
    int (*run)(void);
    void (*close)(void);
} tetris_t;
extern tetris_t tetris;

#endif //TETRIS_TETRIS_H
