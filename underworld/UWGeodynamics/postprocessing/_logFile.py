

class NonLinearBlock(object):
    def __init__(self, string):
        self.string = string
        self.data = dict()
        self.data["Pressure Solve times"] = self.get_vals(["Pressure Solve"], 3)
        self.data["Final V Solve times"] = self.get_vals(["Final V Solve"], 4)
        self.data["Total BSSCR times"] = self.get_vals(["Total BSSCR Linear solve time"], 5)
        self.data["Residuals"] = self.get_vals(["converged", "Residual", "Tolerance"], 5, func=str)
        self.data["Residuals"] = [float(val[:-1]) for val in self.data["Residuals"]]
        self.data["Iterations"] = self.get_vals(["Non linear solver - iteration"], -1, func=int)
        self.data["Solution Time"] = self.get_vals(["solution time"], 5)

    def get_vals(self, FINDSTRING, pos, func=float):
        f = self.string.splitlines()
        vals = [func(line.split()[pos]) for line in f if all([F.lower() in line.lower() for F in FINDSTRING])]
        return vals


class LogFile(object):
    def __init__(self, filename):
        self.filename = filename
        self.nonLinear_blocks = self.get_nonLinear_blocks()
        self.pressure_solve_times = list()
        for obj in self.nonLinear_blocks:
            self.pressure_solve_times += obj.data["Pressure Solve times"]
        self.finalV_solve_times = list()
        for obj in self.nonLinear_blocks:
            self.finalV_solve_times += obj.data["Final V Solve times"]
        self.total_BSSCR_times = list()
        for obj in self.nonLinear_blocks:
            self.total_BSSCR_times += obj.data["Total BSSCR times"]
        self.residuals = list()
        for obj in self.nonLinear_blocks:
            self.residuals += obj.data["Residuals"]
        self.iterations = list()
        for obj in self.nonLinear_blocks:
            self.iterations.append(obj.data["Iterations"][-1])
        self.solution_times = list()
        for obj in self.nonLinear_blocks:
            self.solution_times += obj.data["Solution Time"]

    def get_nonLinear_blocks(self):
        non_linear_blocks = list()
        with open(self.filename, "r") as f:
            block = ""
            inBlock = False
            step = 0
            for line in f:
                if "Non linear solver" in line:
                    inBlock = True
                    step += 1
                if inBlock:
                    if "Converged" not in line:
                        block += line
                    else:
                        block += line
                        inBlock = False
                        block = NonLinearBlock(block)
                        non_linear_blocks.append(block)
                        block = ""
            # Process last potentially non-converged block
            if block:
                block = NonLinearBlock(block)
                non_linear_blocks.append(block)
        self.nonLinear_blocks = non_linear_blocks
        return self.nonLinear_blocks
