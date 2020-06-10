clear; close all; clc;
%% Initialize Workspace
        runNumber = 39;

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


%% Combine the Arduino and DMM data
    
    %Convert the arduino and DMM data into timetables so that they can be
    %passed to the synchronize function
    arduino_Timetable = table2timetable(arduino);
    DMM_Timetable = table2timetable(DMM);
    % Synchronize the two data steams into a timetable called data
    data = synchronize(arduino_Timetable,DMM_Timetable, 'union', 'linear');

%% Smooth these data
    
    %Mass
    mass_Med_Smooth = smoothdata(data.Mass, 'movmedian', 30);
    mass_Smooth=smoothdata(mass_Med_Smooth,'movmean',10);

    mass_Rate_Med_Smooth = smoothdata(data.Mass_Delta, 'movmedian', 80);
    mass_Rate_Smooth = smoothdata(mass_Rate_Med_Smooth, 'movmean', 60);

    %Frequency
    freq_Med_Smooth = smoothdata(data.Frequency, 'movmedian', 10);
    freq_Smooth = smoothdata(freq_Med_Smooth, 'movmean', 30);

    %Temperature
    tower_Temp_Med_Smooth = smoothdata(data.Tower_Temp, 'movmedian', 5);
    tower_Temp_Smooth = smoothdata(tower_Temp_Med_Smooth, 'movmean', 10);

    wash_Temp_Med_Smooth = smoothdata(data.Wash_Temp, 'movmedian', 5);
    wash_Temp_Smooth = smoothdata(wash_Temp_Med_Smooth, 'movmean', 10);

    outlet_Temp_Med_Smooth = smoothdata(data.Outlet_Temp, 'movmedian', 5);
    outlet_Temp_Smooth = smoothdata(outlet_Temp_Med_Smooth, 'movmean', 10);

    resistivity_Med_Smooth = smoothdata(data.Resistance, 'movmedian', 20);
    resistivity_Smooth = smoothdata(resistivity_Med_Smooth, 'movmean', 10);

    
%% Plotting
    %Frequency/Mass/Resistivity vs Time
        close all
        figure
        hold on
        title 'Frequency, Mass, Temperature'
        xlim([0  400])
        xlabel 'Time'

        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            ylim([120000 150000])
            %ylim([0 5000000])
            plot(freq_Smooth, '-b')
            %plot(resistivity_Smooth/5, 'g')
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Temperature'
            ylim([0 125])
            plot(mass_Smooth/25, '-m')
            plot(tower_Temp_Smooth, '-r')
            plot(wash_Temp_Smooth, '-c')
            plot(outlet_Temp_Smooth, '-k')
            plot(mass_Rate_Smooth*100, 'y')
         
        % Legend 
            legend({'Frequency','Mass/15', 'Tower Temperature', 'Wash Temperature','Mass Rate * 100' },'Location','Northwest')
            hold off

    %Frequency Vs Mass
        figure
        hold on
        title 'Frequency vs Mass'
        xlim([0  400])
        xlabel 'Mass'

        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            ylim([108000 144000])
            plot(mass_Smooth, freq_Smooth, '-b')
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Tower Temp'
            ylim([50 120])
            plot(mass_Smooth, tower_Temp_Smooth, '-r')

 legend({'Frequency', 'Tower Temp'},'Location','Northeast')
 hold off
