# Torso noise generation

In order to take stability into account when optimizing, we apply a body force
noise signal $\tilde{f}(t)$ to the torso segment. A stright-forward approach
would be to use a white noise source sampled at simulation time step. The main
problem with this approach is that, if the time step is very small, the
influence of noise signal can be averaged out by dynamics (force is double
integrated to affect position). Therefore a slightly more sophisticated
approach is used: the actually applied noise signal is linearlly interpolated
(First-Order Hold) from a white noise source sampled at a fixed (lower) rate,
specified in another configuration file. Due to the First-Order Hold, the jerk
of force noise can be bounded and thus prevents it from being averaged out.

## How it works

`noise_init()` is called before simulation to read configuration file and
initialize noise generator.

`noise(t)` is called at each time step to provide $\tilde{f}(t)$ using the
method mentioned above.

## configuration file `noise.par`

Text file consisting of the following fields:

*   1 x Character 
    *   `U` : uniform distribution
    *   otherwise : no noise
*   1 x 32-bit integer : random seed
*   2 x float : (mu, sigma) of noise
*   1 x float : sample time
