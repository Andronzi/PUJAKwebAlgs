import Errors from '../config/Errors.js';
import Brush from '../Model/brush.model.js';
class CanvasClasterView {
    constructor(model) {
        var _a, _b, _c, _d, _e, _f, _g;
        this._canvas = (_a = document.querySelector('.canvas')) !== null && _a !== void 0 ? _a : Errors.handleError('null');
        this._canvasContext = (_b = this._canvas.getContext('2d')) !== null && _b !== void 0 ? _b : Errors.handleError('null');
        this._button = (_c = document.querySelector('.sendButton')) !== null && _c !== void 0 ? _c : Errors.handleError('null');
        this._DBSCANButton = (_d = document.querySelector('.DBSCAN')) !== null && _d !== void 0 ? _d : Errors.handleError('null');
        this._Kmeansutton = (_e = document.querySelector('.kmeans')) !== null && _e !== void 0 ? _e : Errors.handleError('null');
        this._rangeInput = (_f = document.querySelector('.range')) !== null && _f !== void 0 ? _f : Errors.handleError('null');
        this._groupInput = (_g = document.querySelector('.groupsize')) !== null && _g !== void 0 ? _g : Errors.handleError('null');
        this._clasterModel = model;
        this._brush = new Brush();
        this._subscribe();
    }
    getMousePosition(event) {
        return {
            x: event.clientX - this._canvas.offsetLeft,
            y: event.clientY - this._canvas.offsetTop
        };
    }
    changeCanvasView() {
        let x = this._clasterModel.positions[this._clasterModel.positions.length - 1];
        let y = this._clasterModel.positions[this._clasterModel.positions.length - 2];
        this._canvasContext.strokeStyle = 'blue';
        this._canvasContext.fillStyle = 'red';
        this._canvasContext.beginPath();
        this._canvasContext.arc(x, y, 5, 0, 2 * Math.PI);
        this._canvasContext.stroke();
        this._canvasContext.fill();
    }
    _subscribe() {
        this._clasterModel.addEventListener('claster.model:addObj', () => this.changeCanvasView());
    }
    handleButtonClick(callback) {
        return this._canvas.addEventListener('click', (event) => {
            event.preventDefault();
            callback(this.getMousePosition(event));
        });
    }
    handleDBSCANFetch(callback) {
        this._DBSCANButton.addEventListener('click', (event) => {
            event.preventDefault();
            callback(parseInt(this._rangeInput.value), parseInt(this._groupInput.value));
        });
    }
    handleKMeansFetch(callback) {
        this._Kmeansutton.addEventListener('click', (event) => {
            event.preventDefault();
            callback();
        });
    }
}
export default CanvasClasterView;
// Почему???
