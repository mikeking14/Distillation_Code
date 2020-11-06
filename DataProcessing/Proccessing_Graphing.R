# --- Install and load Libraries ----
# install.packages("ggplot2")
# install.packages('rlang')
# install.packages("dplyr")
library(ggplot2)
library(dplyr)

# --- Load Data ----
data = read.table("data/Run32/DistillationRun32_Arduino.txt", header = F)
data = subset(data, select = -c(V2,V4,V6,V8,V10,V12,V14,V16,V18,V20,V21,V23,V25,V27) )
colnames(data) <- c('Time','Sec','Heat_Exchanger_Temp','Tower_Temp','Wash_Temp','Outlet_Temp','Mass','Delta_Mass','Frequency','Set_Temp','PID','P','I','D')


# --- Variables ----
time <- data$time
sec <- data$sec
heat_exchanger <-data$heat_exchanger_temp
tower_temp <-data$tower_temp
wash_temp <- data$wash_temp
outlet_temp <- data$outlet_temp
mass <- data$mass
delta_mass <- data$delta_mass
frequency <- data$frequency
set_temp <- data$Set_Temp
PID <- data$PID
P <- data$P
I <- data$I
D <- data$D





# --- Plot ----
plot(sec,frequency,xlab="Seconds",ylab="Frequency", main="Frequency")

# ---- Failed Attempt at GGPLOT ----
# gg <- ggplot(data) + 
#       #Frequency
#       geom_line(x= Sec, y= Frequency/1000, color = 'red', size = .5) + 
#       geom_point(x= Sec, y= Frequency/1000, color = 'red', size = .05) + 
#       
#       #Mass
#       geom_line(x= Sec, y= Mass, color = 'blue', size = .5) + 
#       geom_point(x= Sec, y= Mass, color = 'blue', size = .05) 
# gg
