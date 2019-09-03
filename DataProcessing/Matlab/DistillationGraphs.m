clc;
clear;

%% Import Arduino Data
    % Setup the Import Options
        opts = delimitedTextImportOptions("NumVariables", 25);

    % Specify range and delimiter
        opts.DataLines = [1, Inf];
        opts.Delimiter = ",";

    % Specify column names and types
        opts.VariableNames = ["Time", "VarName2", "Heat_Exchanger_Temp", "VarName4", "Tower_Temp", "VarName6", "Wash_Temp", "VarName8", "Outlet_Temp", "VarName10", "Mass", "VarName12", "Mass_Delta", "VarName14", "Frequency", "VarName16", "Set_Temp", "VarName18", "PID", "VarName20", "P", "VarName22", "I", "VarName24", "D"];
        opts.VariableTypes = ["string", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double", "categorical", "double"];
        opts = setvaropts(opts, 1, "WhitespaceRule", "preserve");
        opts = setvaropts(opts, [1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24], "EmptyFieldRule", "auto");
        opts.ExtraColumnsRule = "ignore";
        opts.EmptyLineRule = "read";

    % Import the data
        arduino = readtable("/Users/michaelking/Documents/MATLAB/Distillation Project/DistillationRun32_Arduino.csv", opts);
        clear opts

 %% Import DMM Data
    % Setup the Import Options
        opts = delimitedTextImportOptions("NumVariables", 2);

    % Specify range and delimiter
        opts.DataLines = [2, Inf];
        opts.Delimiter = ",";

    % Specify column names and types
        opts.VariableNames = ["Time", "Untitled"];
        opts.VariableTypes = ["string", "double"];
        opts = setvaropts(opts, 1, "WhitespaceRule", "preserve");
        opts = setvaropts(opts, 1, "EmptyFieldRule", "auto");
        opts.ExtraColumnsRule = "ignore";
        opts.EmptyLineRule = "read";

    % Import the data
        DMM = readtable("/Users/michaelking/Documents/MATLAB/Distillation Project/DistillationRun32_DMM.csv", opts);
        clear opts

%% Input variables from arduino
    %Create and format datetime variable from arduino
    arduino_Time = table2array(arduino(:,1));
    arduino_Time = extractBefore(arduino_Time,13);
    arduino_Time = datetime(arduino_Time,'InputFormat', 'HH:mm:ss.SSS');
    arduino_Time.Month = 8;
    arduino_Time.Day = 28;
    arduino_Time.Format = 'default';

    %Add the datetime for arduino to the arduino table
    arduino{:,26} = arduino_Time;
    arduino(:,[2,4,6,8,10,12,14,16,18,20,22,24]) = [];
    arduino(:,1) = [];
%% Input variables from DMM
    dmm_Time = table2array(DMM(:,1));
    dmm_Time= datetime(dmm_Time,'InputFormat','yyy-MM-dd HH:mm:ss.SSS');
    dmm_Time.Format = 'default';
    DMM{:,3} = dmm_Time;
    DMM(:,1) = [];
    DMM.Properties.VariableNames({'Untitled', 'Var3'}) = {'Resistivity', 'DMM_Time'};

%% Data Proccessing

% Combine the Arduino and DMM data into one table
    %Convert the arduino and DMM data into timetables so that they can be
    %passed to the synchronize function
    arduino_Timetable = table2timetable(arduino);
    DMM_Timetable = table2timetable(DMM);
    % Synchronize the two data steams into a timetable called data
    data = synchronize(arduino_Timetable,DMM_Timetable, 'union', 'linear');

% Smooth the data
    %Mass
    mass_Med_Smooth = smoothdata(data.Mass, 'movmedian', 30);
    mass_Smooth=smoothdata(mass_Med_Smooth,'movmean',10);

    mass_Rate_Med_Smooth = smoothdata(data.Mass_Delta, 'movmedian', 80)
    mass_Rate_Smooth = smoothdata(mass_Rate_Med_Smooth, 'movmean', 60);

    %Frequency
    freq_Med_Smooth = smoothdata(data.Frequency, 'movmedian', 10)
    freq_Smooth = smoothdata(freq_Med_Smooth, 'movmean', 30);

    %Temperature
    tower_Temp_Med_Smooth = smoothdata(data.Tower_Temp, 'movmedian', 5)
    tower_Temp_Smooth = smoothdata(tower_Temp_Med_Smooth, 'movmean', 10);

    wash_Temp_Med_Smooth = smoothdata(data.Wash_Temp, 'movmedian', 5)
    wash_Temp_Smooth = smoothdata(wash_Temp_Med_Smooth, 'movmean', 10);

    outlet_Temp_Med_Smooth = smoothdata(data.Outlet_Temp, 'movmedian', 5)
    outlet_Temp_Smooth = smoothdata(outlet_Temp_Med_Smooth, 'movmean', 10);

    resistivity_Med_Smooth = smoothdata(data.Resistivity, 'movmedian', 20)
    resistivity_Smooth = smoothdata(resistivity_Med_Smooth, 'movmean', 10);

%% Plotting
    %Frequency/Mass/Resistivity vs Time
        close all
        figure
        hold on
        title 'Frequency, Mass, Temperature'
        xlim([-1000  21000])
        xlabel 'Time'

        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            %ylim([136000 150000])
            ylim([0 5000000])
            plot(freq_Smooth*10, '-b')
            plot(resistivity_Smooth/5, 'g')
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Temperature'
            ylim([0 125])
            plot(mass_Smooth/25, '-m')
            plot(tower_Temp_Smooth, '-r')
            plot(wash_Temp_Smooth, '-c')
            plot(outlet_Temp_Smooth, '-k')
            plot(mass_Rate_Smooth*100, 'y')
            legend({'Frequency','Resistivity','Mass/15', 'Tower Temperature', 'Wash Temperature','Mass Rate * 100' },'Location','Northwest')
            hold off

    %Frequency Vs Mass
        figure
        hold on
        title 'Frequency vs Mass'
        xlim([0  1600])
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

        %Vertical Lines
            xl = xline(80,'--k');
            xl.LabelVerticalAlignment = 'middle';
            xl = xline(1495,'--k');
            xl.LabelVerticalAlignment = 'middle';

 legend({'Frequency', 'Tower Temp'},'Location','Northeast')
 hold off
