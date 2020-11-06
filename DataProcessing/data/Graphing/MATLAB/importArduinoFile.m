function arduino = importArduinoFile_old(filename, dataLines)
%IMPORTFILE Import data from a text file
%  Arduino = IMPORTARDUINOFILE(FILENAME) reads data from text
%  file FILENAME for the default selection.  Returns the data as a table.
%
%  DMM = IMPORTARDUINOFILE(FILE, DATALINES) reads data for
%  the specified row interval(s) of text file FILENAME. Specify
%  DATALINES as a positive scalar integer or a N-by-2 array of positive
%  scalar integers for dis-contiguous row intervals.
%
%  Example:
%  Arduino = importarduinofile(filename, [1, Inf]);
%
%  See also READTABLE.

%% Input handling

% If dataLines is not specified, define defaults
if nargin < 2
    dataLines = [1, Inf];
end

%% Setup the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 44);

% Specify range and delimiter
opts.DataLines = [2, Inf];
opts.Delimiter = ["\t", ">"];

% Specify column names and types
opts.VariableNames = ["Time", "Var2", "Var3", "SetPosition", "Var5", "CurrentPossition", "Var7", "PID_Error", "Var9", "PID", "Var11", "P", "Var13", "I", "Var15", "D", "Var17", "Room_Temp", "Var19", "Wash_Temp", "Var21", "Outlet_Temp", "Var23", "Tower_Temp", "Var25", "Mass", "Var27", "Mass_Delta", "Var29", "Frequency", "Var31", "Set_Temp", "Var33", "Set_Temp_Counter", "Var35", "CheckPoint", "Var37", "State", "Var39", "kP", "Var41", "kI", "Var43", "kD"];
opts.SelectedVariableNames = ["Time", "SetPosition", "CurrentPossition", "PID_Error", "PID", "P", "I", "D", "Room_Temp", "Wash_Temp", "Outlet_Temp", "Tower_Temp", "Mass", "Mass_Delta", "Frequency", "Set_Temp", "Set_Temp_Counter", "CheckPoint", "State", "kP", "kI", "kD"];
opts.VariableTypes = ["datetime", "string", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double", "string", "double"];

% Specify file level properties
opts.ImportErrorRule = "omitrow";
opts.MissingRule = "omitrow";
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";
opts.ConsecutiveDelimitersRule = "join";

% Specify variable properties
opts = setvaropts(opts, ["Var2", "Var3", "Var5", "Var7", "Var9", "Var11", "Var13", "Var15", "Var17", "Var19", "Var21", "Var23", "Var25", "Var27", "Var29", "Var31", "Var33", "Var35", "Var37", "Var39", "Var41", "Var43"], "WhitespaceRule", "preserve");
opts = setvaropts(opts, ["Var2", "Var3", "Var5", "Var7", "Var9", "Var11", "Var13", "Var15", "Var17", "Var19", "Var21", "Var23", "Var25", "Var27", "Var29", "Var31", "Var33", "Var35", "Var37", "Var39", "Var41", "Var43"], "EmptyFieldRule", "auto");
opts = setvaropts(opts, "Time", "InputFormat", "HH:mm:ss.SSS");

% Import the data
arduino = readtable(filename, opts);

clear opts
end