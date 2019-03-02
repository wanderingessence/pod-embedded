# Badgerloop Pod IV
*Developers: Ezra Boley, Eric Udlis, Rohan Daruwala, Luke Houge, Alex Vesel*

The master repository for Badgerloop's pod in the Hyperloop Competition

## Beaglebone Make Instructions

There are currently two sets of targets:

1) Making the main programs (`badgerloop_LV` and `badgerloop_HV`)

```
make
```

2) Making the examples/tests

```
make examples
```

### Adding Tests

In order to add a test or example, put a .c or .cpp file into the respective "example" folder. Make sure that your file contains a "main" function.