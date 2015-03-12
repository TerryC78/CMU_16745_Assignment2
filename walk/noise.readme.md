## How it works

`noise_init()` is called before simulation to read configuration file and
initialize noise generator.

`noise(t)` is called at each time step to provide a scalar noise, which is
linearly interpolated from a discrete-time random series drawn from the
distribution specified in confguration file.

## configuration file `noise.par`

Text file consisting of the following fields:

*   1 x Character 
    *   `U` : uniform noise
    *   otherwise : no noise
*   1 x 32-bit integer : random seed
*   2 x float : (mu, sigma) of noise
*   1 x float : sample time
