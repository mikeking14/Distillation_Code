clear; close all; clc;
%% Initialize Workspace
        runNumber = 43;

%% Import Arduino Data
        
        % Specify Filepath
                
        s1 = pwd;
        s1 = s1(1:size(s1,2)-15);
        s2 = sprintf('Run%d/DistillationRun%d_Arduino.txt',runNumber,runNumber);
        Arduinofilename = horzcat(s1,s2);
        
        
        % Import Arduino data
        arduino = importArduinoFile(Arduinofilename);
        

%% Import DMM Data
       
        % Specify Filepath
        
        s2 = sprintf('Run%d/DistillationRun%d_DMM.txt',runNumber,runNumber);
        DMMfilename = horzcat(s1,s2);
        % Import DMM data
        DMM = importDMMfile(DMMfilename);
       
        
%% Data Preparation 
 
    mass_Med_Smooth = smoothdata(arduino.Mass, 'movmedian', 30);
    mass_Smooth=smoothdata(mass_Med_Smooth,'movmean',10);

    mass_Rate_Med_Smooth = smoothdata(arduino.Mass_Delta, 'movmedian', 80);
    mass_Rate_Smooth = smoothdata(mass_Rate_Med_Smooth, 'movmean', 60);

    %Frequency
    freq_Med_Smooth = smoothdata(arduino.Frequency, 'movmedian', 10);
    freq_Smooth = smoothdata(freq_Med_Smooth, 'movmean', 30);

    %Temperature
    tower_Temp_Med_Smooth = smoothdata(arduino.Tower_Temp, 'movmedian', 5);
    tower_Temp_Smooth = smoothdata(tower_Temp_Med_Smooth, 'movmean', 10);

    wash_Temp_Med_Smooth = smoothdata(arduino.Wash_Temp, 'movmedian', 5);
    wash_Temp_Smooth = smoothdata(wash_Temp_Med_Smooth, 'movmean', 10);

    outlet_Temp_Med_Smooth = smoothdata(arduino.Outlet_Temp, 'movmedian', 5);
    outlet_Temp_Smooth = smoothdata(outlet_Temp_Med_Smooth, 'movmean', 10);
    
    
%% Plotting - ALL DATA
    
    %Frequency/Mass/Resistivity vs Time
        close all
        figure
        hold on
        title 'Frequency, Mass, Temperature'
        xlabel 'Time'

        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            ylim([128000 134000])
            plot(freq_Smooth, '-b')
            %plot(resistivity_Smooth/5, 'g')
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Temperature'
            ylim([0 125])
            plot(arduino.Mass/20, '-m')
            plot(arduino.Set_Temp, '--r')
            plot(arduino.Tower_Temp, '-r')
            plot(wash_Temp_Smooth, '-c')
            plot(outlet_Temp_Smooth, '-k')
            plot(mass_Rate_Smooth*100, '-y')
         
        % Legend 
            legend({'Frequency','Mass/20', 'Set Temperature', 'Tower Temperature','Wash Temp','Outlet Temp', 'Mass Rate * 100' },'Location','Northwest')
            hold off

            
%% Plotting - PID Control
        
        figure
        hold on
        title 'PID CONTROL'
        xlabel 'Time'
        

        
        yyaxis left
        ylabel 'Temperature'
        plot(arduino.Set_Temp/2, '--red')
        plot(arduino.Tower_Temp, '-red')
        plot(arduino.State*20, '--blue')
        x=1:size(arduino);
        y=50;
        plot(x,y*ones(size(x)))
        
        
        
        yyaxis right
        ylabel 'PID Variables'
        ylim([-200 400])
        
        plot(arduino.PID)
        %plot(arduino.PID_Error,'black')
        %plot(arduino.P, '-m')
        %plot(arduino.I, '-b')
        %plot(arduino.D * 100, 'c')
        
        
        
        legend({'Set Temp','Tower Temp', 'State', 'Line', 'PID', 'PID ERROR','P','I' }, 'Location', 'Northwest')

