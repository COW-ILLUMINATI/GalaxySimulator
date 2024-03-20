# GalaxySimulator
A very basic N-Body simulator for visualizing galaxy collisions.

This simulator was made for my astrophysics project. Do not use it -- it sucks.

Run ``./creator.out`` to configure the simulator.
You will need to use the "Prepare Universe" option to add custom galaxies to the simulation, it is not done automatically.

Run ``./simulator.out`` to simulate the system. If no window is created and you are using Nvidia hardware, ``chmod +x`` the program and try running it through Proton Experimental.

Build with ``make``


Controls:
Arrow keys to rotate
Page up/down to zoom
Space to play/pause
Enter to change focus

For best results:
	- Use low timesteps -- this will allow for the simulator to take more samples of every interaction and improve paths. [Linear]
	- Use high budgets  -- this will improve feild estimation, giving a better approximation of the shape of the system. [Quadratic]
	- Use a high radius -- this will improve floating-point precision. Don't go *too* high though! [Linear]
	- Make the galaxy's mass no more than 25% of the black hole -- the initial velocities used in galaxy creation use elliptic curve approximation and are not designed for exotic ratios. This could be changed in the code for your specific ratios. [Free]

You should expect to have to timelapse your simulations.
