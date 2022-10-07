:

# -m1 is a dummy rotator - this will allow gsc to run without any hardware

# The 8080 & 8081 come from the default.cfg file
#   azimuth-port = 8080
#   elevation-port = 8081
#

rotctld -v -v -v -Z -m1 -t8080 &
rotctld -v -v -v -Z -m1 -t8081 & 

wait
