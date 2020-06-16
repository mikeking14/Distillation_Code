
mass = arduino_Timetable.Mass;

lagged_mass = zeros(size(mass));
lagged_mass(2:end) = mass(1:end-1);

change = mass - lagged_mass;

change(abs(change) > 15) = 0;
plot(cumsum(change))