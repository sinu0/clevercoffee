// Mock Data Module for Testing
// Usage: Add ?mock=true to the URL to enable mock mode

const MOCK_DATA = {
    parameters: [
        // PID Controller
        {
            name: 'pid.enabled',
            displayName: 'PID Enabled',
            value: 1,
            type: 0, // boolean
            min: 0,
            max: 1,
            show: true,
            hasHelpText: false,
            reboot: false
        },
        // Temperature
        {
            name: 'brew.setpoint',
            displayName: 'Brew Temperature',
            value: 93.5,
            type: 3, // float
            min: 80,
            max: 110,
            show: true,
            hasHelpText: true,
            reboot: false
        },
        {
            name: 'STEAM_MODE',
            displayName: 'Steam Mode',
            value: 0,
            type: 0, // boolean
            min: 0,
            max: 1,
            show: true,
            hasHelpText: false,
            reboot: false
        },
        {
            name: 'BACKFLUSH_ON',
            displayName: 'Backflush Mode',
            value: 0,
            type: 0, // boolean
            min: 0,
            max: 1,
            show: true,
            hasHelpText: false,
            reboot: false
        },
        // Scale functions
        {
            name: 'TARE_ON',
            displayName: 'Tare Scale',
            value: 0,
            type: 0,
            min: 0,
            max: 1,
            show: true,
            hasHelpText: false,
            reboot: false
        },
        {
            name: 'CALIBRATION_ON',
            displayName: 'Scale Calibration',
            value: 0,
            type: 0,
            min: 0,
            max: 1,
            show: true,
            hasHelpText: false,
            reboot: false
        }
    ],
    currentTemperature: 92.3
};

// Mock API responses
window.MockAPI = {
    isMockMode: false,
    currentTemperature: MOCK_DATA.currentTemperature,
    temperatureIntervalId: null,
    
    init() {
        const urlParams = new URLSearchParams(window.location.search);
        this.isMockMode = urlParams.get('mock') === 'true';
        
        if (this.isMockMode) {
            console.log('🎭 Mock mode enabled - using test data');
            this.startTemperatureSimulation();
        }
        
        return this.isMockMode;
    },
    
    async getParameters(filter = '', offset = 0, limit = 5) {
        if (!this.isMockMode) return null;
        
        // Simulate network delay
        await new Promise(resolve => setTimeout(resolve, 100));
        
        let filteredParams = [...MOCK_DATA.parameters];
        
        // Apply filter if provided (basic implementation)
        if (filter) {
            // Filter logic would go here - for now, return all parameters
            // In real implementation, this would filter by parameter category/section
        }
        
        // Simple pagination
        const start = offset;
        const end = offset + limit;
        const paginatedParams = filteredParams.slice(start, end);
        
        return {
            parameters: paginatedParams
        };
    },
    
    async postParameters(formData) {
        if (!this.isMockMode) return null;
        
        // Simulate network delay
        await new Promise(resolve => setTimeout(resolve, 200));
        
        console.log('📝 Mock: Parameters saved', formData);
        
        return {
            success: true,
            message: 'Parameters saved successfully (mock)'
        };
    },
    
    async toggleFunction(endpoint, paramName) {
        if (!this.isMockMode) return null;
        
        // Simulate network delay
        await new Promise(resolve => setTimeout(resolve, 150));
        
        // Find and toggle the parameter
        const param = MOCK_DATA.parameters.find(p => p.name === paramName);
        if (param) {
            param.value = param.value === 1 ? 0 : 1;
            console.log(`🔄 Mock: Toggled ${paramName} to ${param.value}`);
        }
        
        return {
            success: true,
            value: param ? param.value : 0
        };
    },
    
    startTemperatureSimulation() {
        // Clear any existing interval
        if (this.temperatureIntervalId) {
            clearInterval(this.temperatureIntervalId);
        }
        
        // Simulate temperature fluctuation
        this.temperatureIntervalId = setInterval(() => {
            // Get brew setpoint from parameters
            const brewParam = MOCK_DATA.parameters.find(p => p.name === 'brew.setpoint');
            const setpoint = brewParam ? brewParam.value : 93.5;
            
            // Vary temperature slightly around setpoint (±1°C)
            const variation = (Math.random() - 0.5) * 2;
            this.currentTemperature = setpoint - 1.2 + variation; // Slightly below setpoint
            
            // Update display if element exists
            const tempElement = document.getElementById('varTEMP');
            if (tempElement) {
                tempElement.textContent = this.currentTemperature.toFixed(1);
            }
        }, 2000);
    },
    
    stopTemperatureSimulation() {
        if (this.temperatureIntervalId) {
            clearInterval(this.temperatureIntervalId);
            this.temperatureIntervalId = null;
        }
    },
    
    getTemperature() {
        return this.currentTemperature.toFixed(1);
    }
};

// Initialize mock mode on load
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => window.MockAPI.init());
} else {
    window.MockAPI.init();
}
