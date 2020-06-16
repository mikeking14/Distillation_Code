clear; close all; clc;
%% Initialize Workspace
        runNumber = 40;

%% Import Arduino Data
        
        % Specify Filepath
        str = "/Users/michaelking/Documents/PlatformIO/Projects/Distillation_Code/DataProcessing/data/Run";
        str2 = num2str(runNumber);
        str3 = "/DistillationRun";
        str4 = "_Arduino.txt";
        
        % Import Arduino data
        Arduinofilename = append(str,str2,str3,str2,str4);
        arduino = importArduinoFile(Arduinofilename);
        

%% Import DMM Data
       
        % Specify Filepath
        str5 = "_DMM.txt";
        DMMfilename = append(str,str2,str3,str2,str5);
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
            plot(arduino.SetTemp, '--r')
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
        
        
        %yyaxis left
        ylabel 'Temperature'
        plot(arduino.SetTemp, '--red')
        plot(arduino.Tower_Temp, '-red')
        legend({'Set Temp','Tower Temp'})

%         yyaxis right
%         ylabel 'PID Variables'
%         ylim([-100 300])
%         plot(arduino.PID)
%         plot(arduino.PID_Error,'black')
%         plot(arduino.P, '-m')
%         plot(arduino.I, '-b')
%         plot(arduino.D, 'c')
%         
%         legend({'Set Temp','Tower Temp', 'PID', 'PID ERROR','P','I', 'D' },'Location','Northwest')

