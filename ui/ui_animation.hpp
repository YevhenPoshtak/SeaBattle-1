/*
 * Battleship 1 Game Project
 * Group: Compmath 2
 * Author: Poshtak
 * 
 * File: ui_animation.hpp
 * Description: Header file for UI animation effects. Defines the UIAnimation class
 *              with methods for various animated sequences including victory/defeat
 *              animations, menu backgrounds, and decorative ship animations.
 */

#ifndef UI_ANIMATION_HPP
#define UI_ANIMATION_HPP

class UIAnimation {
public:
    // Display end-game animation with underwater scene
    // playerWon: true for victory fireworks, false for defeat
    static void drawFirework(bool playerWon);
    
    // Draw animated main menu background with submarine and ocean elements
    // frame: current animation frame for motion calculations
    static void drawMenuAnimation(int frame);
    
    // Draw animated ships at bottom of screen with combat sequence
    // frame: current animation frame, startY: vertical position, maxX: screen width
    static void drawBottomShipAnimation(int frame, int startY, int maxX);
};

#endif